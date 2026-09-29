/**
 * @file formation_courtesy_advisor.cpp
 * @brief Formation courtesy advisor responsibilities.
 *
 * @details This file implements formation courtesy advisor behavior for tiered
 * decision making and action arbitration. It records the declarations,
 * settings, fixtures, or guidance needed by that responsibility. Its
 * package-relative location is
 * `src/decision/advisors/social/formation_courtesy_advisor.cpp`.
 */
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <semaforr/decision/advisors/social/formation_courtesy_advisor.hpp>
#include <semaforr/decision/advisors/social/social_prediction.hpp>
#include <semaforr/domain/motion_model.hpp>
#include <stdexcept>
#include <utility>
#include <vector>

namespace semaforr::decision {
namespace {

/**
 * @brief Validates positive for this subsystem.
 *
 * Arguments:
 * - @p value: Supplies value input to the operation.
 * - @p name: Supplies name input to the operation.
 *
 * Returns:
 * - No value; effects are applied to owned state or outputs.
 *
 * Exceptions:
 * - None documented; validation or dependency failures may propagate.
 */
void validatePositive(double value, std::string_view name) {
  if (!std::isfinite(value) || value <= 0.0) {
    throw std::invalid_argument(std::string(name) + " must be finite and positive");
  }
}

/**
 * @brief Performs the formation radius operation for this subsystem.
 *
 * Arguments:
 * - @p formation: Supplies formation input to the operation.
 * - @p formation_index: Supplies formation index input to the operation.
 * - @p pedestrians: Supplies pedestrians input to the operation.
 *
 * Returns:
 * - `double` containing the operation result.
 *
 * Exceptions:
 * - None documented; validation or dependency failures may propagate.
 */
double formationRadius(
    const domain::FormationObservation& formation, std::size_t formation_index,
    const std::vector<domain::PedestrianObservation>& pedestrians) {
  double radius = 0.0;
  for (const auto& pedestrian : pedestrians) {
    if (pedestrian.formation_index && *pedestrian.formation_index == formation_index) {
      radius = std::max(radius, euclideanDistance(formation.center, pedestrian.position));
    }
  }
  return radius;
}

}  // namespace

/**
 * @brief Performs the formation courtesy advisor operation for this
 * subsystem.
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
FormationCourtesyAdvisor::FormationCourtesyAdvisor(FormationCourtesyAdvisorConfiguration configuration) : configuration_(std::move(configuration)) {
  if (configuration_.advisor_name.empty()) {
    throw std::invalid_argument(
        "formation courtesy advisor name must not be empty");
  }
  if (configuration_.maximum_age < std::chrono::nanoseconds::zero()) {
    throw std::invalid_argument(
        "formation courtesy advisor maximum age must be non-negative");
  }
  if (!std::isfinite(configuration_.minimum_confidence) ||
      configuration_.minimum_confidence < 0.0 ||
      configuration_.minimum_confidence > 1.0) {
    throw std::invalid_argument("formation courtesy advisor minimum confidence must be within [0, 1]");
  }
  if (!std::isfinite(configuration_.minimum_formation_confidence) ||
      configuration_.minimum_formation_confidence < 0.0 ||
      configuration_.minimum_formation_confidence > 1.0) {
    throw std::invalid_argument("formation courtesy advisor minimum formation confidence must be within [0, 1]");
  }
  validatePositive(configuration_.courtesy_margin_m, "formation courtesy margin");
  if (!std::isfinite(configuration_.weight)) {throw std::invalid_argument("formation courtesy advisor weight must be finite");
  }
}

/**
 * @brief Evaluates package content for this subsystem.
 *
 * Arguments:
 * - @p context: Supplies context input to the operation.
 * - @p candidates: Supplies candidates input to the operation.
 *
 * Returns:
 * - `AdvisorEvaluation` containing the operation result.
 *
 * Exceptions:
 * - None documented; validation or dependency failures may propagate.
 */
AdvisorEvaluation FormationCourtesyAdvisor::evaluate(const DecisionContext& context, std::span<const domain::Action> candidates) const {
  AdvisorEvaluation evaluation;
  evaluation.weight = configuration_.weight;
  evaluation.explanation = "prefer actions that avoid crossing a social formation";

  // no action space, no evaluation
  if (!context.action_space) {
    return evaluation;
  }

  // no crowd, no evaluation
  const auto& current = context.world.crowd.current();
  if (!current || !current->usable(configuration_.maximum_age,
                                    configuration_.minimum_confidence)) {
    evaluation.explanation =
        "social data absent, invalid, or stale; advisor disabled";
    return evaluation;
  }

  std::vector<std::size_t> qualifying;
  for (std::size_t index = 0U; index < current->formations.size(); ++index) {
    if (current->formations[index].confidence >=
        configuration_.minimum_formation_confidence) {
      qualifying.push_back(index);
    }
  }
  if (qualifying.empty()) {
    evaluation.explanation = "no confident formation evidence available; advisor disabled";
    return evaluation;
  }

  evaluation.model_revision_used = static_cast<std::size_t>(current->observed_at.count());
  evaluation.participated = true;
  evaluation.scores.reserve(candidates.size());

  const domain::Point2D start = context.world.robot.pose.position;
  constexpr int samples = 20;

  for (const auto& action : candidates) {
    const domain::Point2D end =
        domain::expectedPoseAfterAction(context.world.robot.pose, action, *context.action_space).position;

    double penalty = 0.0;
    for (const std::size_t index : qualifying) {
      const auto& formation = current->formations[index];
      const double keep_out_radius = formationRadius(formation, index, current->pedestrians) + configuration_.courtesy_margin_m;

      double minimum_distance = euclideanDistance(start, formation.center);
      for (int sample = 1; sample <= samples; ++sample) {
        const double fraction = static_cast<double>(sample) / static_cast<double>(samples);
        const domain::Point2D robot = interpolate(start, end, fraction);
        minimum_distance = std::min(minimum_distance, euclideanDistance(robot, formation.center));
      }

      if (minimum_distance < keep_out_radius) {
        penalty += formation.confidence * (keep_out_radius - minimum_distance);
      }
    }
    evaluation.scores.push_back({action, -penalty});
  }
  return evaluation;
}

}  // namespace semaforr::decision
