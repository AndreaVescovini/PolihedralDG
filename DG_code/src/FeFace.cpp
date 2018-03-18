#include "FeFace.hpp"

namespace dgfem
{

FeFace::FeFace(unsigned order, unsigned dofNo,
               const std::vector<std::array<unsigned, 3>>& basisComposition,
               const QuadRule<Eigen::Vector2d>& triaRule)
  : dofNo_{dofNo}, basisComposition_{basisComposition}, triaRule_{triaRule} {}

unsigned FeFace::getDofNo() const
{
  return dofNo_;
}

}
