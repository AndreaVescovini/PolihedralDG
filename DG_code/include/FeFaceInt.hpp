#ifndef _FE_FACE_INT_HPP_
#define _FE_FACE_INT_HPP_

#include "FeFace.hpp"
#include "FaceInt.hpp"
#include "QuadRuleManager.hpp"
#include <vector>
#include <array>
#include "geom.hpp"
#include <Eigen/Dense>

namespace dgfem
{

class FeFaceInt : public FeFace
{
public:
  using TheFace = geom::FaceInt;

  FeFaceInt(const TheFace& face, unsigned order, unsigned dofNo,
            const std::vector<std::array<unsigned, 3>>& basisComposition,
            const QuadRuleManager::Rule2D& triaRule);

  geom::real getPhi(unsigned side, unsigned p, unsigned f) const;
  const Eigen::Vector3d& getPhiDer(unsigned side, unsigned p, unsigned f) const;

  void printBasis(std::ostream& out) const override;
  void printBasisDer(std::ostream& out) const override;

  virtual ~FeFaceInt() = default;

private:
  const TheFace& face_;

// Auxiliary function that fills phi_ and phiDer_
  void compute_basis() override;

// Auxiliary function that, given the side of the face, the quadrature point p
// and basis function f, returns the index in which the corresponding value
// is stored in phi_ and phiDer_
  unsigned sub2ind(unsigned side, unsigned p, unsigned f) const;

};

}

#endif // _FE_FACE_INT_HPP_
