/**
 * @file formation_courtesy_advisor.hpp
 * @brief Formation courtesy advisor responsibilities.
 *
 * @details This file defines formation courtesy advisor behavior for tiered
 * decision making and action arbitration. It centers on
 * `FormationCourtesyAdvisorConfiguration`, `FormationCourtesyAdvisor`. Its
 * package-relative location is
 * `include/semaforr/decision/advisors/social/formation_courtesy_advisor.hpp`.
 */
#ifndef SEMAFORR_DECISION_ADVISORS_SOCIAL_FORMATION_COURTESY_ADVISOR_HPP
#define SEMAFORR_DECISION_ADVISORS_SOCIAL_FORMATION_COURTESY_ADVISOR_HPP

#include <chrono>
#include <semaforr/decision/advisor.hpp>
#include <string>
#include <string_view>

namespace semaforr::decision {

/**
 * @brief Encapsulates formation courtesy advisor configuration state and
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
struct FormationCourtesyAdvisorConfiguration {
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
  double minimum_formation_confidence{0.5};
  double courtesy_margin_m{0.5};
  double weight{1.0};
  std::string advisor_name{"formation_courtesy"};
};

/**
 * @brief Encapsulates formation courtesy advisor state and behavior for
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
class FormationCourtesyAdvisor final : public Advisor {
 public:
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
  explicit FormationCourtesyAdvisor(
      FormationCourtesyAdvisorConfiguration configuration);

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
    return {"live_crowd_observations", "social_formations"};
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
            "prefer actions that avoid crossing a social formation"};
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
  FormationCourtesyAdvisorConfiguration configuration_;
};

}  // namespace semaforr::decision

#endif  // SEMAFORR_DECISION_ADVISORS_SOCIAL_FORMATION_COURTESY_ADVISOR_HPP
