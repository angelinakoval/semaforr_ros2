/**
 * @file social_prediction.hpp
 * @brief Social prediction responsibilities.
 *
 * @details This file defines social prediction behavior for tiered decision making
 * and action arbitration. It records the declarations, settings, fixtures,
 * or guidance needed by that responsibility. Its package-relative location
 * is `include/semaforr/decision/advisors/social/social_prediction.hpp`.
 */
#ifndef SEMAFORR_DECISION_ADVISORS_SOCIAL_SOCIAL_PREDICTION_HPP
#define SEMAFORR_DECISION_ADVISORS_SOCIAL_SOCIAL_PREDICTION_HPP

#include <semaforr/domain/social.hpp>

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
domain::Point2D interpolate(const domain::Point2D& from, const domain::Point2D& to, double fraction);

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
double euclideanDistance(const domain::Point2D& left, const domain::Point2D& right);

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
domain::Point2D pedestrianAt(const domain::PedestrianObservation& pedestrian, domain::SocialTimestamp observed_at, double seconds);

}  // namespace semaforr::decision

#endif  // SEMAFORR_DECISION_ADVISORS_SOCIAL_SOCIAL_PREDICTION_HPP
