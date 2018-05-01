#include "Mesh.hpp"
#include "FeSpace.hpp"
#include "MeshReaderPoly.hpp"
#include "Problem.hpp"
#include "Operators.hpp"
#include "ExprOperators.hpp"
#include "Watch.hpp"

#include <cmath>

using namespace PolyDG;

int main()
{
  std::string fileName = "../meshes/cube_str48h.mesh";

  PolyDG::MeshReaderPoly reader;
  PolyDG::Mesh Th(fileName, reader);

  unsigned r = 2;
  FeSpace Vh(Th, r);

  PhiI v;
  GradPhiJ uGrad;
  GradPhiI vGrad;
  JumpPhiJ uJump;
  JumpPhiI vJump;
  AverGradPhiJ uGradAver;
  AverGradPhiI vGradAver;
  PenaltyScaling gamma(10.0);
  Normal n;

  Function f([](Eigen::Vector3d x) {
    return -std::exp(x(0)*x(1)*x(2)) * (x(0)*x(0)*x(1)*x(1) +
                                        x(1)*x(1)*x(2)*x(2) +
                                        x(0)*x(0)*x(2)*x(2) ); });
  Function gd([](Eigen::Vector3d x) { return std::exp(x(0)*x(1)*x(2)); });

  Problem sym(Vh);
  Problem nonsym(Vh);

  Timings::Watch ch;

  sym.integrateVol(dot(uGrad, vGrad), true);
  sym.integrateFacesInt(-dot(uGradAver, vJump)-dot(uJump, vGradAver)+gamma*dot(uJump, vJump), true);
  sym.integrateFacesExt(-dot(uGradAver, vJump)-dot(uJump, vGradAver)+gamma*dot(uJump, vJump), 1,  true);
  // nonsym.integrateVol(dot(uGrad, vGrad), false);
  // nonsym.integrateFacesInt(-dot(uGradAver, vJump)-dot(uJump, vGradAver)+gamma*dot(uJump, vJump), false);
  // nonsym.integrateFacesExt(-dot(uGradAver, vJump)-dot(uJump, vGradAver)+gamma*dot(uJump, vJump), 1, false);

  sym.integrateVolRhs(f * v);
  sym.integrateFacesExtRhs(-gd * dot(n, vGrad) + gamma * gd * v, 1);
  ch.start();
  sym.finalizeMatrix();

  ch.stop();
  std::cout << ch << std::endl;

  std::cout << sym.getRhs() << std::endl;
  std::cout << sym.getMatrix()/*.selfadjointView<Eigen::Upper>()*/ << std::endl;
  // std::cout << nonsym.getMatrix() << std::endl;

  return 0;
}
