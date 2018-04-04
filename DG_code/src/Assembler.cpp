#include "Assembler.hpp"

namespace dgfem
{

Assembler::Assembler(const FeSpace& Vh)
  : Vh_{Vh},
    A_{Vh.getFeElementsNo() * Vh.getDofNo(), Vh.getFeElementsNo() * Vh.getDofNo()},
    b_{Eigen::VectorXd::Zero(Vh.getFeElementsNo() * Vh.getDofNo())} {}

void Assembler::printMatrix(std::ostream& out) const
{
  out << A_ << std::endl;
}

void Assembler::printMatrixSym(std::ostream& out) const
{
  out << A_.selfadjointView<Eigen::Upper>() << std::endl;
}

void Assembler::printRhs(std::ostream& out) const
{
  out << b_ << std::endl;
}


} // namespace dgfem
