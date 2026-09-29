/**
 * @file predicted_social_veto.cpp
 * @brief Predicted social veto responsibilities.
 *
 * @details This file implements predicted social veto behavior for tiered decision
 * making and action arbitration. It records the declarations, settings,
 * fixtures, or guidance needed by that responsibility. Its
 * package-relative location is `src/decision/predicted_social_veto.cpp`.
 */
#include <algorithm>
#include <cmath>
#include <semaforr/decision/advisors/social/social_prediction.hpp>
#include <semaforr/decision/predicted_social_veto.hpp>
#include <semaforr/domain/motion_model.hpp>
#include <stdexcept>
#include <utility>

namespace semaforr::decision {

/**
 * @brief Performs the predicted social veto operation for this subsystem.
 *
 * Arguments:
 * - @p configuration: Supplies configuration input to the operation.
 *
 * Returns:
 * - No value; effects are applied to owned state or outputs.
 *
 * Exceptions:
 * - None documented; validation or dependency failures may propagate.
 */
PredictedSocialVeto::PredictedSocialVeto(
    PredictedSocialVetoConfiguration configuration)
    : configuration_(std::move(configuration)) {
  if (configuration_.maximum_age < std::chrono::nanoseconds::zero()) {
    throw std::invalid_argument(
        "predicted social veto maximum age must be non-negative");
  }
  if (!std::isfinite(configuration_.minimum_confidence) ||
      configuration_.minimum_confidence < 0.0 ||
      configuration_.minimum_confidence > 1.0) {
    throw std::invalid_argument(
        "predicted social veto minimum confidence must be within [0, 1]");
  }
  if (!std::isfinite(configuration_.prediction_horizon_s) ||
      configuration_.prediction_horizon_s <= 0.0) {
    throw std::invalid_argument(
        "predicted social veto prediction horizon must be finite and " "positive");
  }
  if (!std::isfinite(configuration_.minimum_separation_m) ||
      configuration_.minimum_separation_m <= 0.0) {
    throw std::invalid_argument(
        "predicted social veto minimum separation must be finite and " "positive");
  }
}

/**
 * @brief Evaluates package content for this subsystem.
 *
 * Arguments:
 * - @p context: Supplies context input to the operation.
 *
 * Returns:
 * - `std::vector<Veto>` containing the operation result.
 *
 * Exceptions:
 * - None documented; validation or dependency failures may propagate.
 */
std::vector<Veto> PredictedSocialVeto::evaluate(const DecisionContext& context) const {
  // no action space, no vetoes
  std::vector<Veto> vetoes;
  if (!context.action_space) {
    return vetoes;
  }

  // no crowd, no vetoes
  const auto& current = context.world.crowd.current();
  if (!current || !current->usable(configuration_.maximum_age,
                                    configuration_.minimum_confidence)) {
    return vetoes;
  }

  const domain::Point2D start = context.world.robot.pose.position;
  const double observation_age_s =
      std::chrono::duration<double>(current->data_age).count();
  constexpr int samples = 20;

  for (const auto& action : context.viable_actions) {
    const domain::Point2D end = domain::expectedPoseAfterAction(context.world.robot.pose, action,*context.action_space).position;

    bool vetoed = false;
    for (const auto& pedestrian : current->pedestrians) {
      if (pedestrian.confidence < configuration_.minimum_confidence) {
        continue;
      }
      double minimum_separation = euclideanDistance(
          start, pedestrianAt(pedestrian, current->observed_at,
                              observation_age_s));
      for (int sample = 1; sample <= samples; ++sample) {
        const double fraction =
            static_cast<double>(sample) / static_cast<double>(samples);
        const domain::Point2D robot = interpolate(start, end, fraction);
        const domain::Point2D person = pedestrianAt(
            pedestrian, current->observed_at,
            observation_age_s +
                configuration_.prediction_horizon_s * fraction);
        minimum_separation =
            std::min(minimum_separation, euclideanDistance(robot, person));
      }
      if (minimum_separation < configuration_.minimum_separation_m) {
        vetoed = true;
        break;
      }
    }
    if (vetoed) {
      vetoes.push_back({action, "PredictedSocialVeto",
                        "predicted_social_veto:minimum_separation",
                        RejectionKind::Cognitive, VetoCategory::Unsafe});
    }
  }
  return vetoes;
}

}  // namespace semaforr::decision
