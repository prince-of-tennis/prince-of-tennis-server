#ifndef COURT_CHECK_H
#define COURT_CHECK_H

#include "common/util/point_3d.h"

#define COURT_HALF_WIDTH 4.115f
#define COURT_HALF_LENGTH 11.89f
bool is_in_court(Point3d p);

#endif // COURT_CHECK_H