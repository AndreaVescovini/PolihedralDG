#include "FeFaceInt.hpp"
#include "Legendre.hpp"

#include <cmath>
#include <algorithm>

namespace PolyDG
{

FeFaceInt::FeFaceInt(const TheFace& face, unsigned order, unsigned dofNo,
                     const std::vector<std::array<unsigned, 3>>& basisComposition,
                     const QuadRule<Eigen::Vector2d>& triaRule)
  : FeFace(dofNo, basisComposition, triaRule), face_{face}
{
  compute_basis();
  penaltyParam_ = order * order / std::min(face.getTet1().getPoly().getDiameter(),
                                          face.getTet2().getPoly().getDiameter());
}

void FeFaceInt::compute_basis()
{
  // hb and hb2 contain the half of the dimensions of the bounding box of the two
  // polyhedra sharing the face. They are needed for the computation of the scaled Legendre polynomials.
  Eigen::Vector3d hb = face_.getTet1().getPoly().getBoundingBox().sizes() / 2;
  Eigen::Vector3d hb2 = face_.getTet2().getPoly().getBoundingBox().sizes() / 2;

  // mb and mb2 contain the center of the bounding box of the two polyhedra sharing
  // the face. They are needed for the computation of the scaled Legendre polynomials.
  Eigen::Vector3d mb = face_.getTet1().getPoly().getBoundingBox().center();
  Eigen::Vector3d mb2 = face_.getTet2().getPoly().getBoundingBox().center();

  unsigned quadPointsNo = triaRule_.getPointsNo();

  phi_.reserve(2 * quadPointsNo * dofNo_);
  phiDer_.reserve(2 * quadPointsNo * dofNo_);

  // loop over quadrature points
  for(unsigned p = 0; p < quadPointsNo; p++)
  {
    // I map the quadrature point from the refrence triangle to the face of the
    // reference tetrahedron and then to the physical one, finally I rescale it
    // in order to compute the scaled legendre polynomial.
    Eigen::Vector3d physicPt = (face_.getTet1().getMap() * (QuadRuleManager::getFaceMap(face_.getFaceNoTet1()) *
                                                                          triaRule_.getPoint(p).homogeneous()) - mb).array() / hb.array();
    Eigen::Vector3d physicPt2 = (face_.getTet1().getMap() * (QuadRuleManager::getFaceMap(face_.getFaceNoTet1()) *
                                                                           triaRule_.getPoint(p).homogeneous()) - mb2).array() / hb2.array();

    // Loop over basis functions
    for(unsigned f = 0; f < dofNo_; f++)
    {
      std::array<std::array<Real, 2>, 3> polval;
      std::array<std::array<Real, 2>, 3> polval2;

      // Loop over the three coordinates
      for(unsigned i = 0; i < 3; i++)
      {
        // I compute the basis for both the sides of the face
        polval[i] = basis::legendre(basisComposition_[f][i], physicPt(i));
        polval2[i] = basis::legendre(basisComposition_[f][i], physicPt2(i));

        // I rescale the results in order to have the scaled Legendre polynomials
        polval[i][0] /= std::sqrt(hb(i));
        polval[i][1] /= (std::sqrt(hb(i)) * hb(i));
        polval2[i][0] /= std::sqrt(hb2(i));
        polval2[i][1] /= (std::sqrt(hb2(i)) * hb2(i));
      }

      phi_.emplace_back(polval[0][0] * polval[1][0] * polval[2][0]);
      phi_.emplace_back(polval2[0][0] * polval2[1][0] * polval2[2][0]);

      phiDer_.emplace_back(polval[0][1] * polval[1][0] * polval[2][0],
                           polval[0][0] * polval[1][1] * polval[2][0],
                           polval[0][0] * polval[1][0] * polval[2][1]);
      phiDer_.emplace_back(polval2[0][1] * polval2[1][0] * polval2[2][0],
                           polval2[0][0] * polval2[1][1] * polval2[2][0],
                           polval2[0][0] * polval2[1][0] * polval2[2][1]);
    }
  }
}

void FeFaceInt::printBasis(std::ostream& out = std::cout) const
{
  out << "Face " << face_.getId() << '\n';
  out << face_.getVertex(0).getCoords().transpose() << " - " << face_.getVertex(1).getCoords().transpose() << " - " << face_.getVertex(2).getCoords().transpose() << '\n';

  std::array<SideType, 2> sides = { Out, In };

  // Loop over sides
  for(unsigned i = 0; i < 2; i++)
  {

    // Loop over basis functions
    for(unsigned f = 0; f < dofNo_; f++)
    {

      // Loop over quadrature points
      for(unsigned p = 0; p < triaRule_.getPointsNo(); p++)
        out << getPhi(sides[i], p, f) << ' ';

      out << '\n';
    }
    out << '\n';
  }
  out << '\n';
}

void FeFaceInt::printBasisDer(std::ostream& out = std::cout) const
{
  out << "Face " << face_.getId() << '\n';
  out << face_.getVertex(0).getCoords().transpose() << " - " << face_.getVertex(1).getCoords().transpose() << " - " << face_.getVertex(2).getCoords().transpose() << '\n';

  std::array<SideType, 2> sides = { Out, In };

  // Loop over sides
  for(unsigned i = 0; i < 2; i++)
  {

    // Loop over basis functions
    for(unsigned f = 0; f < dofNo_; f++)
    {

      // Loop over quadrature points
      for(unsigned p = 0; p < triaRule_.getPointsNo(); p++)
        out << getPhiDer(sides[i], p, f).transpose() << '\n';

      out << '\n';
    }
    out << '\n';
  }
  out << '\n';
}

} // namespace PolyDG
