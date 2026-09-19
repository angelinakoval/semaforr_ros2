/**
 * @file approach_direction_advisor.hpp
 * @brief Approach direction advisor responsibilities.
 *
 * @details This file defines approach direction advisor behavior for tiered
 * decision making and action arbitration. It centers on
 * `ApproachDirectionAdvisorConfiguration`, `ApproachDirectionAdvisor`. Its
 * package-relative location is
 * `include/semaforr/decision/advisors/social/approach_direction_advisor.hpp`.
 */
#ifndef SEMAFORR_DECISION_ADVISORS_SOCIAL_APPROACH_DIRECTION_ADVISOR_HPP
#define SEMAFORR_DECISION_ADVISORS_SOCIAL_APPROACH_DIRECTION_ADVISOR_HPP

#include <chrono>
#include <semaforr/decision/advisor.hpp>
#include <string>
#include <string_view>

namespace semaforr::decision {

/**
 * @brief Encapsulates approach direction advisor configuration state and
 * behavior for this subsystem.
 *
 * Arguments:
 * - None.
 *
 * Returns:
 * - Not applicable to this declaration.
 *
 * Exceptions:
 * - None documented; validation or dependency failures may propagate.
 */
struct ApproachDirectionAdvisorConfiguration {
  /**
   * @brief Performs the milliseconds operation for this subsystem.
   *
   * Arguments:
   * - @p argument_1: Supplies argument 1 input to the operation.
   *
   * Returns:
   * - `std::chrono::nanoseconds maximum_age{` containing the operation
   * result.
   *
   * Exceptions:
   * - None documented; validation or dependency failures may propagate.
   */
  std::chrono::nanoseconds maximum_age{std::chrono::milliseconds(750)};
  double minimum_confidence{0.25};
  double approach_radius_m{2.0};
  double weight{1.0};
  std::string advisor_name{"approach_direction"};
};

/**
 * @brief Encapsulates approach direction advisor state and behavior for
 * this subsystem.
 *
 * Arguments:
 * - None.
 *
 * Returns:
 * - Not applicable to this declaration.
 *
 * Exceptions:
 * - None documented; validation or dependency failures may propagate.
 */
class ApproachDirectionAdvisor final : public Advisor {
 public:
  /**
   * @brief Performs the approach direction advisor operation for this
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
  explicit ApproachDirectionAdvisor(
      ApproachDirectionAdvisorConfiguration configuration);

  /**
   * @brief Performs the name operation for this subsystem.
   *
   * Arguments:
   * - None.
   *
   * Returns:
   * - `std::string_view` containing the operation result.
   *
   * Exceptions:
   * - None documented; validation or dependency failures may propagate.
   */
  std::string_view name() const noexcept override {
    return configuration_.advisor_name;
  }

  /**
   * @brief Performs the dependencies operation for this subsystem.
   *
   * Arguments:
   * - None.
   *
   * Returns:
   * - `std::vector<std::string_view>` containing the operation result.
   *
   * Exceptions:
   * - None documented; validation or dependency failures may propagate.
   */
  std::vector<std::string_view> dependencies() const override {
    return {"live_crowd_observations", "pedestrian_facing"};
  }

  /**
   * @brief Performs the metadata operation for this subsystem.
   *
   * Arguments:
   * - None.
   *
   * Returns:
   * - `AdvisorMetadata` containing the operation result.
   *
   * Exceptions:
   * - None documented; validation or dependency failures may propagate.
   */
  AdvisorMetadata metadata() const override {
    return {dependencies(),
            {domain::ActionType::Pause, domain::ActionType::Forward,
             domain::ActionType::TurnLeft, domain::ActionType::TurnRight},
            true,
            ScoreNormalization::TenPoint,
            "prefer actions that approach a tracked person from where "
            "they can see the robot coming, not from behind"};
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
  AdvisorEvaluation evaluate(
      const DecisionContext& context,
      std::span<const domain::Action> candidates) const override;

 private:
  ApproachDirectionAdvisorConfiguration configuration_;
};

}  // namespace semaforr::decision

#endif  // SEMAFORR_DECISION_ADVISORS_SOCIAL_APPROACH_DIRECTION_ADVISOR_HPP
