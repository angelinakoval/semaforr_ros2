/**
 * @file sudden_proximity_mandate.hpp
 * @brief Sudden proximity mandate responsibilities.
 *
 * @details This file defines sudden proximity mandate behavior for tiered decision
 * making and action arbitration. It centers on
 * `SuddenProximityMandateConfiguration`, `SuddenProximityMandate`. Its
 * package-relative location is
 * `include/semaforr/decision/sudden_proximity_mandate.hpp`.
 */
#ifndef SEMAFORR_DECISION_SUDDEN_PROXIMITY_MANDATE_HPP
#define SEMAFORR_DECISION_SUDDEN_PROXIMITY_MANDATE_HPP

#include <chrono>
#include <semaforr/decision/rules.hpp>

namespace semaforr::decision {

/**
 * @brief Encapsulates sudden proximity mandate configuration state and
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
struct SuddenProximityMandateConfiguration {
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
  double emergency_distance_m{0.4};
};

/**
 * @brief Encapsulates sudden proximity mandate state and behavior for this
 * subsystem.
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
class SuddenProximityMandate final : public MandatoryRule {
 public:
  /**
   * @brief Performs the sudden proximity mandate operation for this
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
  explicit SuddenProximityMandate(
      SuddenProximityMandateConfiguration configuration);

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
    return "SuddenProximityMandate";
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
    return {"live_crowd_observations"};
  }

  /**
   * @brief Evaluates package content for this subsystem.
   *
   * Arguments:
   * - @p context: Supplies context input to the operation.
   *
   * Returns:
   * - `std::optional<Decision>` containing the operation result.
   *
   * Exceptions:
   * - None documented; validation or dependency failures may propagate.
   */
  std::optional<Decision> evaluate(
      const DecisionContext& context) const override;

 private:
  SuddenProximityMandateConfiguration configuration_;
};

}  // namespace semaforr::decision

#endif  // SEMAFORR_DECISION_SUDDEN_PROXIMITY_MANDATE_HPP
