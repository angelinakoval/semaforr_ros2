/**
 * @file predicted_social_veto.hpp
 * @brief Predicted social veto responsibilities.
 *
 * @details This file defines predicted social veto behavior for tiered decision
 * making and action arbitration. It centers on
 * `PredictedSocialVetoConfiguration`, `PredictedSocialVeto`. Its
 * package-relative location is
 * `include/semaforr/decision/predicted_social_veto.hpp`.
 */
#ifndef SEMAFORR_DECISION_PREDICTED_SOCIAL_VETO_HPP
#define SEMAFORR_DECISION_PREDICTED_SOCIAL_VETO_HPP

#include <chrono>
#include <semaforr/decision/rules.hpp>

namespace semaforr::decision {

/**
 * @brief Encapsulates predicted social veto configuration state and
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
struct PredictedSocialVetoConfiguration {
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
  double prediction_horizon_s{2.0};
  double minimum_separation_m{0.5};
};

/**
 * @brief Encapsulates predicted social veto state and behavior for this
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
class PredictedSocialVeto final : public VetoRule {
 public:
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
  explicit PredictedSocialVeto(PredictedSocialVetoConfiguration configuration);

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
    return "PredictedSocialVeto";
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
   * - `std::vector<Veto>` containing the operation result.
   *
   * Exceptions:
   * - None documented; validation or dependency failures may propagate.
   */
  std::vector<Veto> evaluate(const DecisionContext& context) const override;

 private:
  PredictedSocialVetoConfiguration configuration_;
};

}  // namespace semaforr::decision

#endif  // SEMAFORR_DECISION_PREDICTED_SOCIAL_VETO_HPP
