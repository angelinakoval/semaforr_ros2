/**
 * @file social_prediction.cpp
 * @brief Social prediction responsibilities.
 *
 * @details This file implements social prediction behavior for tiered decision
 * making and action arbitration. It records the declarations, settings,
 * fixtures, or guidance needed by that responsibility. Its
 * package-relative location is
 * `src/decision/advisors/social/social_prediction.cpp`.
 */
#include <algorithm>
#include <chrono>
#include <cmath>
#include <semaforr/decision/advisors/social/social_prediction.hpp>

namespace semaforr::decision {

/**
 * @brief Performs the interpolate operation for this subsystem.
 *
 * Arguments:
 * - @p from: Supplies from input to the operation.
 * - @p to: Supplies to input to the operation.
 * - @p fraction: Supplies fraction input to the operation.
 *
 * Returns:
 * - `domain::Point2D` containing the operation result.
 *
 * Exceptions:
 * - None documented; validation or dependency failures may propagate.
 */
domain::Point2D interpolate(const domain::Point2D& from, const domain::Point2D& to, double fraction) {
  return {
    from.x_m + (to.x_m - from.x_m) * fraction,
    from.y_m + (to.y_m - from.y_m) * fraction};
}

/**
 * @brief Performs the euclidean distance operation for this subsystem.
 *
 * Arguments:
 * - @p left: Supplies left input to the operation.
 * - @p right: Supplies right input to the operation.
 *
 * Returns:
 * - `double` containing the operation result.
 *
 * Exceptions:
 * - None documented; validation or dependency failures may propagate.
 */
double euclideanDistance(const domain::Point2D& left, const domain::Point2D& right) {
  return std::hypot(left.x_m - right.x_m, left.y_m - right.y_m);
}

/**
 * @brief Performs the pedestrian at operation for this subsystem.
 *
 * Arguments:
 * - @p pedestrian: Supplies pedestrian input to the operation.
 * - @p observed_at: Supplies observed at input to the operation.
 * - @p seconds: Supplies seconds input to the operation.
 *
 * Returns:
 * - `domain::Point2D` containing the operation result.
 *
 * Exceptions:
 * - None documented; validation or dependency failures may propagate.
 */
domain::Point2D pedestrianAt(const domain::PedestrianObservation& pedestrian, domain::SocialTimestamp observed_at, double seconds) {
  const auto target = observed_at + std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(seconds));

  if (!pedestrian.predicted_trajectory.empty()) {
    domain::Point2D previous_position = pedestrian.position;
    domain::SocialTimestamp previous_time = observed_at;
    for (const auto& prediction : pedestrian.predicted_trajectory) {
      if (target <= prediction.predicted_at) {
        const double span = std::chrono::duration<double>(prediction.predicted_at - previous_time).count();
        const double elapsed = std::chrono::duration<double>(target - previous_time).count();
        return interpolate(previous_position, prediction.position, std::clamp(elapsed / span, 0.0, 1.0));
      }
      previous_position = prediction.position;
      previous_time = prediction.predicted_at;
    }
    return pedestrian.predicted_trajectory.back().position;
  }
  return {pedestrian.position.x_m + pedestrian.velocity_mps.x_m * seconds,
          pedestrian.position.y_m + pedestrian.velocity_mps.y_m * seconds};
}

}  // namespace semaforr::decision
