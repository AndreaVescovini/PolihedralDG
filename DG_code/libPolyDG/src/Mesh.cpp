/*!
    @file   Mesh.cpp
    @author Andrea Vescovini
    @brief  Implementation for the class Mesh
*/

#include "Face.hpp"
#include "Mesh.hpp"
#include "Watch.hpp"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iomanip>
#include <limits>
#include <memory>
#include <random>
#include <unordered_set>
#include <vector>

namespace PolyDG
{

Mesh::Mesh(const std::string& fileName, MeshReader& reader)
{
  #ifdef VERBOSITY
    std::cout << "Reading the mesh..................";
    Utilities::Watch ch;
    ch.start();
  #endif

  reader.read(*this, fileName);

  #ifdef VERBOSITY
    ch.stop();
    std::cout << "Done!   " << ch << std::endl;
    ch.reset();
  #endif

  if(facesInt_.size() == 0)
  {
    #ifdef VERBOSITY
      std::cout << "Finding internal faces............";
      ch.start();
    #endif

    computeFaces();

    #ifdef VERBOSITY
      ch.stop();
      std::cout << "Done!   " << ch << std::endl;
      ch.reset();
    #endif
  }

  #ifdef VERBOSITY
    std::cout << "Computing diameters of elements...";
    ch.start();
  #endif

  computeDiameters();

  #ifdef VERBOSITY
    ch.stop();
    std::cout << "Done!   " << ch << std::endl;
  #endif
}

void Mesh::printAll(std::ostream& out) const
{
  this->print(std::max(vertices_.size(), tetrahedra_.size()), out);
}

void Mesh::printHead(std::ostream& out) const
{
  this->print(5, out);
}

void Mesh::printInfo(std::ostream& out) const
{
  out << "--------- MESH INFO ---------" << '\n';
  out << "Vertices: \t\t" << vertices_.size() << '\n';
  out << "Elements (polyhedra): \t" << polyhedra_.size() << '\n';
  out << "Tetrahedra: \t\t" << tetrahedra_.size() << '\n';
  out << "External faces: \t" << facesExt_.size() << '\n';
  out << "Internal faces: \t" << facesInt_.size() << '\n';
  out << "Maximum diameter =  " << hmax_ << '\n';
  out << "Minimum diameter =  " << hmin_ <<  '\n';
  out << "Ratio hmax / hmin =  \t" << hmax_ / hmin_ << '\n';
  out << "-----------------------------" << std::endl;
}

void Mesh::computeFaces()
{
  std::unordered_set<std::unique_ptr<Face>> temp;
  temp.reserve(tetrahedra_.size() * 4);
  facesInt_.reserve(tetrahedra_.size() * 2);

  for(Tetrahedron& t : tetrahedra_)
    for(unsigned faceNo = 0; faceNo < 4; faceNo++)
    {
      // I store faces in temp, the constructor will sort vertices so that
      // the comparison will be easy, if a face is not present it will be inserted,
      // otherwise we will get res.second == false.

      // The i-th face is that one without the (3-i)-th vertex.
      auto res = temp.emplace(new Face(t.getVertex(static_cast<unsigned>(faceNo < 1)),
                                       t.getVertex(static_cast<unsigned>(faceNo < 2) + 1),
                                       t.getVertex(static_cast<unsigned>(faceNo < 3) + 2),
                                       t, 3 - faceNo));
      if(res.second == false)
      {
        // If I find a face of a tetrahedron of a different polyhedron I insert it in
        // facesInt_, then in any case I erase the face from temp
        const unsigned elem1 = t.getPoly().getId();
        const unsigned elem2 = (*res.first)->getTetIn().getPoly().getId();

        if(elem1 != elem2)
        {
          if(elem1 > elem2)
            facesInt_.emplace_back((*res.first)->getVertex(0), (*res.first)->getVertex(1), (*res.first)->getVertex(2),
                                   (*res.first)->getTetIn(), (*res.first)->getFaceNoTetIn(), t);
          else
            facesInt_.emplace_back((*res.first)->getVertex(0), (*res.first)->getVertex(1), (*res.first)->getVertex(2),
                                    t, 3 - faceNo, (*res.first)->getTetIn());
        }
        temp.erase(res.first);
      }
    }

  facesInt_.shrink_to_fit();

  // Now in temp there are only external faces that have been inserted only once,
  // I loop over external faces and I find in the faces stored in temp the
  // information about the tetrahedron to which it belongs and the local number
  // of the face.
  for(FaceExt& f : facesExt_)
  {
    std::unique_ptr<Face> fext(new Face(f));
    auto got = temp.find(fext);

    f.setTetIn((*got)->getTetIn());
    f.setFaceNoTetIn((*got)->getFaceNoTetIn());
    f.checkNormalSign();

    temp.erase(got);
  }

}

void Mesh::computeDiameters()
{
  // I add in each polyhedron the vertices that are on the faces.
  // I will use them in order to compute the bounding box and the diameter.

  hmax_ = 0.0;
  hmin_ = std::numeric_limits<Real>::max();

  for(Polyhedron& p : polyhedra_)
  {
    if(p.getDiameter() > hmax_)
      hmax_ = p.getDiameter();
    if(p.getDiameter() < hmin_)
      hmin_ = p.getDiameter();
  }
}

void Mesh::print(SizeType lineNo, std::ostream& out) const
{
  out << "-------- MESH --------\n";

  out << "VERTICES: " << vertices_.size() << '\n';
  for(SizeType i = 0; i < std::min(lineNo, vertices_.size()); i++)
    out << vertices_[i] << '\n';

  out << "\nTETRAHEDRA: " << tetrahedra_.size() << '\n';
  for(SizeType i = 0; i < std::min(lineNo, tetrahedra_.size()); i++)
    out << tetrahedra_[i] << '\n';

  out << "\nEXTERNAL FACES: " << facesExt_.size() << '\n';
  for(SizeType i = 0; i < std::min(lineNo, facesExt_.size()); i++)
    out << facesExt_[i] << '\n';

  out << "\nINTERNAL FACES: " << facesInt_.size() << '\n';
  for(SizeType i = 0; i < std::min(lineNo, facesInt_.size()); i++)
    out << facesInt_[i] << '\n';

  out << "\nPOLYHEDRA: " << polyhedra_.size() << '\n';
  for(SizeType i = 0; i < std::min(lineNo, polyhedra_.size()); i++)
    out << polyhedra_[i] << '\n';

  out << "----------------------" << std::endl;
}

void Mesh::exportMeshVTK(const std::string& fileName, unsigned precision) const
{
  #ifdef VERBOSITY
    std::cout << "Exporting the mesh................";
    Utilities::Watch ch;
    ch.start();
  #endif

  std::ofstream fout;

  if(fileName.substr(fileName.size() - 4, 4) != ".vtu")
    fout.open(fileName + ".vtu");
  else
    fout.open(fileName);

  // Create a vector with random integers in order to distinguish elements.
  std::vector<unsigned> elemValues;
  elemValues.reserve(polyhedra_.size());
  for(unsigned i = 0; i < polyhedra_.size(); i++)
    elemValues.emplace_back(i);

  std::default_random_engine dre;
  std::shuffle(elemValues.begin(), elemValues.end(), dre);

  // Print the header
  fout << "<?xml version=\"1.0\"?>\n";
  fout << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\">\n";
  fout << "  <UnstructuredGrid>\n";

  for(auto it = polyhedra_.cbegin(); it != polyhedra_.cend(); it++)
  {
    const std::vector<std::reference_wrapper<const Vertex>> nodes(it->verticesCbegin(), it->verticesCend());

    fout << "    <Piece NumberOfPoints=\"" << nodes.size() << "\" NumberOfCells=\"" << it->getTetrahedraNo() << "\">\n";

    fout << std::setprecision(precision) << std::scientific;

    // Print the nodes coordinates.
    fout << "      <Points>\n";
    fout << "        <DataArray type=\"Float64\" NumberOfComponents=\"3\" format=\"ascii\">\n         ";
    for(auto itNod = nodes.cbegin(); itNod != nodes.cend(); itNod++)
      fout << ' ' << itNod->get().getX() << ' ' << itNod->get().getY() << ' ' << itNod->get().getZ();
    fout << "\n        </DataArray>\n";
    fout << "      </Points>\n";

    // Print the cells (tetrahedra) connectivity and type.
    fout << "      <Cells>\n";
    fout << "        <DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">\n         ";
    for(SizeType i = 0; i < it->getTetrahedraNo(); i++) // Loop over tetrahedra
      for(SizeType j = 0; j < 4; j++) // Loop over vertices
        fout << ' ' << (std::find(nodes.cbegin(), nodes.cend(), it->getTetra(i).getVertex(j)) - nodes.cbegin());
    fout << "\n        </DataArray>\n";

    fout << "         <DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">\n         ";
    for(unsigned offset = 4; offset <= it->getTetrahedraNo() * 4; offset += 4)
      fout << ' ' << offset;
    fout << "\n        </DataArray>\n";

    fout << "        <DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n         ";
    for(unsigned i = 0; i < it->getTetrahedraNo(); i++)
      fout << " 10";
    fout << "\n        </DataArray>\n";
    fout << "      </Cells>\n";

    // Print a value for the Polyhedron.
    fout << "      <CellData Scalars=\"Mesh\">\n";
    fout << "        <DataArray type=\"UInt32\" Name=\"Mesh\" format=\"ascii\">\n         ";
    for(SizeType i = 0; i < it->getTetrahedraNo(); i++)
      fout << ' ' << elemValues[it->getId()];
    fout << "\n        </DataArray>\n";
    fout << "      </CellData>\n";

    fout << "    </Piece>\n";
  }

  fout << "  </UnstructuredGrid>\n";
  fout << "</VTKFile>" << std::endl;

  fout.close();

  #ifdef VERBOSITY
    ch.stop();
    std::cout << "Done!   " << ch << std::endl;
  #endif
}

MeshFormatError::MeshFormatError(const std::string& what_arg)
  : std::runtime_error(what_arg) {}

MeshFormatError::MeshFormatError(const char* what_arg)
  : std::runtime_error(what_arg) {}

} // namespace PolyDG
