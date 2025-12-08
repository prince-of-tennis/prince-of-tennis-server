#include "court_check.h"

bool is_in_court(Point3d p)
{
    if (p.x < -COURT_HALF_WIDTH || p.x > COURT_HALF_WIDTH)
    {
        return false;
    }

    if (p.z < -COURT_HALF_LENGTH || p.z > COURT_HALF_LENGTH)
    {
        return false;
    }

    return true;
}