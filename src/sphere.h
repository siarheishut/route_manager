#ifndef ROOT_MANAGER_SRC_SPHERE_H_
#define ROOT_MANAGER_SRC_SPHERE_H_

namespace rm::sphere {
struct Coords {
  double latitude, longitude;
};

// Both lhs and rhs coords are in radians.
double CalculateDistance(Coords lhs, Coords rhs);
}

#endif // ROOT_MANAGER_SRC_SPHERE_H_
