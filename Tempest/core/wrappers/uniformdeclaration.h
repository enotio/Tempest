#ifndef UNIFORMDECLARATION_H
#define UNIFORMDECLARATION_H

#include <Tempest/AbstractAPI>

namespace Tempest{

class Texture2d;

class UniformDeclaration{
  private:
    std::vector<int>  type;
    std::vector<char> name;
  public:
    UniformDeclaration& add(const char* name, Tempest::Decl::ComponentType t){
      this->name.insert( this->name.end(), name, name+strlen(name));
      this->name.push_back(0);
      type.push_back(t);
      return *this;
      }
    UniformDeclaration& add(const char* name, Tempest::Decl::UniformType t){
      this->name.insert( this->name.end(), name, name+strlen(name));
      this->name.push_back(0);
      type.push_back(t);
      return *this;
      }

  friend class AbstractShadingLang;
  friend class ProgramObject;
  };

}

#endif // UNIFORMDECLARATION_H
