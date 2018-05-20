#include "Mesh.hpp"
#include "FeSpace.hpp"
#include "MeshReaderPoly.hpp"
#include "Problem.hpp"
#include "Operators.hpp"
#include "ExprOperators.hpp"
#include "Watch.hpp"

#include <cmath>
#include <exception>

int main()
{
  // Exact solution and source term
  auto uex = [](const Eigen::Vector3d& x) { return std::exp(x(0) * x(1) * x(2)); };
  auto source = [&uex](const Eigen::Vector3d& x) { return -uex(x) * (x(0) * x(0) * x(1) * x(1) +
                                                                     x(1) * x(1) * x(2) * x(2) +
                                                                     x(0) * x(0) * x(2) * x(2));};
  auto uexGrad = [&uex](const Eigen::Vector3d& x) -> Eigen::Vector3d { return uex(x)*Eigen::Vector3d(x(1) * x(2),
                                                                                                     x(0) * x(2),
                                                                                                     x(0) * x(1)); };

  // Mesh reading
  std::string fileName = "../meshes/cube_str1296p.mesh";

  PolyDG::MeshReaderPoly reader;
  PolyDG::Mesh Th(fileName, reader);
  Th.printInfo();

  // FeSpace Creation
  unsigned r = 2;
  PolyDG::FeSpace Vh(Th, r);

  // Operators
  PolyDG::PhiI            v;
  PolyDG::GradPhiJ        uGrad;
  PolyDG::GradPhiI        vGrad;
  PolyDG::JumpPhiJ        uJump;
  PolyDG::JumpPhiI        vJump;
  PolyDG::AverGradPhiJ    uGradAver;
  PolyDG::AverGradPhiI    vGradAver;
  PolyDG::PenaltyScaling  gamma(10.0);
  PolyDG::Normal          n;
  PolyDG::Function        f(source), gd(uex);

  // Problem instantation and integration
  std::cout << "-------------------- Symmetric Problem --------------------" << std::endl;
  PolyDG::Problem poisson(Vh);

  poisson.integrateVol(dot(uGrad, vGrad), true);
  poisson.integrateFacesExt(-dot(uGradAver, vJump) - dot(uJump, vGradAver) + gamma * dot(uJump, vJump), 1, true);
  poisson.integrateFacesInt(-dot(uGradAver, vJump) - dot(uJump, vGradAver) + gamma * dot(uJump, vJump), true);
  poisson.integrateVolRhs(f * v);
  poisson.integrateFacesExtRhs(-gd * dot(n, vGrad) + gamma * gd * v, 1);

  poisson.finalizeMatrix();

  Timings::Watch ch;

  // LU
  std::cout << "\nSolving with SparseLU..." << std::endl;
  ch.start();
  poisson.solveLU();
  ch.stop();
  std::cout << "L2 error  = " << poisson.computeErrorL2(uex) << std::endl;
  std::cout << "H10 error = " << poisson.computeErrorH10(uexGrad) << std::endl;
  std::cout << ch << std::endl;

  ch.reset();

  // Chlolesky
  std::cout << "\nSolving with SparseCholesky..." << std::endl;
  ch.start();
  poisson.solveCholesky();
  ch.stop();
  std::cout << "L2 error  = " << poisson.computeErrorL2(uex) << std::endl;
  std::cout << "H10 error = " << poisson.computeErrorH10(uexGrad) << std::endl;
  std::cout << ch << std::endl;

  ch.reset();

  // Conjugate Gradient
  std::cout << "\nSolving with ConjugateGradient..." << std::endl;
  ch.start();
  poisson.solveCG(Eigen::VectorXd::Zero(poisson.getDim()), 2 * poisson.getDim(), 1e-10);
  ch.stop();
  std::cout << "L2 error  = " << poisson.computeErrorL2(uex) << std::endl;
  std::cout << "H10 error = " << poisson.computeErrorH10(uexGrad) << std::endl;
  std::cout << ch << std::endl;

  ch.reset();

  // BiCGSTAB
  std::cout << "\nSolving with BiCGSTAB..." << std::endl;
  ch.start();
  poisson.solveBiCGSTAB(Eigen::VectorXd::Zero(poisson.getDim()), 2 * poisson.getDim(), 1e-10);
  ch.stop();
  std::cout << "L2 error  = " << poisson.computeErrorL2(uex) << std::endl;
  std::cout << "H10 error = " << poisson.computeErrorH10(uexGrad) << std::endl;
  std::cout << ch << std::endl;

  // Another problem
  std::cout << "------------------ Non Symmetric Problem ------------------" << std::endl;
  poisson.clearMatrix();
  poisson.clearRhs();

  PolyDG::Stiff stiff;
  PolyDG::Mass  mass;

  poisson.integrateVol(stiff + mass, true);
  poisson.integrateFacesExt(-dot(uGradAver, vJump) + dot(uJump, vGradAver) + gamma * dot(uJump, vJump), 1,  false);
  poisson.integrateFacesInt(-dot(uGradAver, vJump) + dot(uJump, vGradAver) + gamma * dot(uJump, vJump), false);

  poisson.integrateVolRhs(f * v);
  poisson.integrateFacesExtRhs(gd * dot(n, vGrad) + gamma * gd * v, 1);

  poisson.finalizeMatrix();

  // LU
  std::cout << "\nSolving with SparseLU..." << std::endl;
  ch.start();
  poisson.solveLU();
  ch.stop();
  std::cout << "L2 error  = " << poisson.computeErrorL2(uex) << std::endl;
  std::cout << "H10 error = " << poisson.computeErrorH10(uexGrad) << std::endl;
  std::cout << ch << std::endl;

  ch.reset();

  // Chlolesky
  std::cout << "\nSolving with SparseCholesky..." << std::endl;
  ch.start();
  try
  {
    poisson.solveCholesky();
    ch.stop();
    std::cout << "L2 error  = " << poisson.computeErrorL2(uex) << std::endl;
    std::cout << "H10 error = " << poisson.computeErrorH10(uexGrad) << std::endl;
    std::cout << ch << std::endl;

  } catch(const std::exception&e)
  {
    std::cout << e.what() << std::endl;
  }

  ch.reset();

  // Conjugate Gradient
  std::cout << "\nSolving with ConjugateGradient..." << std::endl;
  ch.start();
  try
  {
    poisson.solveCG(Eigen::VectorXd::Zero(poisson.getDim()), 2 * poisson.getDim(), 1e-10);
    ch.stop();
    std::cout << "L2 error  = " << poisson.computeErrorL2(uex) << std::endl;
    std::cout << "H10 error = " << poisson.computeErrorH10(uexGrad) << std::endl;
    std::cout << ch << std::endl;
  } catch(const std::exception& e)
  {
    std::cout << e.what() << std::endl;
  }

  ch.reset();

  // BiCGSTAB
  std::cout << "\nSolving with BiCGSTAB..." << std::endl;
  ch.start();
  poisson.solveBiCGSTAB(Eigen::VectorXd::Zero(poisson.getDim()), 2 * poisson.getDim(), 1e-10);
  ch.stop();
  std::cout << "L2 error  = " << poisson.computeErrorL2(uex) << std::endl;
  std::cout << "H10 error = " << poisson.computeErrorH10(uexGrad) << std::endl;
  std::cout << ch << std::endl;

  std::cout << "-----------------------------------------------------------" << std::endl;

  return 0;
}
