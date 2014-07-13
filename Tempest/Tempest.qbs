import qbs

Product {
  name: 'Tempest'
  type: "dynamiclibrary"

  destinationDirectory: "../lib"

  Properties {
    condition: qbs.toolchain.contains("gcc") || qbs.toolchain.contains("mingw")
    cpp.cxxFlags :["-std=c++11"]
    }

  Depends { name: "cpp" }

  Group {
    name: "core"
    prefix:"core/"
    files: [
      "*.cpp",
      "*.h"
      ]
    }

  Group {
    name: "core.wrappers"
    prefix:"core/wrappers/"
    files: [
      "*.cpp",
      "*.h"
      ]
    }

  Group {
    name: "dataControl"
    prefix:"dataControl/"
    files: [
      "*.cpp",
      "*.h"
      ]
    }

  Group {
    name: "2d"
    prefix:"2d/"
    files: [
      "*.cpp",
      "*.h"
      ]
    }

  Group {
    name: "dx"
    prefix:"dx/"
    files: [
      "*.cpp",
      "*.h"
      ]
    }

  Group {
    name: "ogl"
    prefix:"ogl/"
    files: [
      "*.cpp",
      "*.h"
      ]
    }

  Group {
    name: "io"
    prefix:"io/"
    files: [
      "*.cpp",
      "*.h"
      ]
    }

  Group {
    name: "math"
    prefix:"math/"
    files: [
      "*.cpp",
      "*.h"
      ]
    }

  Group {
    name: "scene"
    prefix:"scene/"
    files: [
      "*.cpp",
      "*.h"
      ]
    }

  Group {
    name: "shading"
    prefix:"shading/"
    files: [
      "*.cpp",
      "*.h"
      ]
    }

  Group {
    name: "system"
    prefix:"system/"
    files: [
      "*.cpp",
      "*.h"
      ]
    }

  Group {
    name: "ui"
    prefix:"ui/"
    files: [
      "*.cpp",
      "*.h"
      ]
    }

  Group {
    name: "utils"
    prefix:"utils/"
    files: [
      "*.cpp",
      "*.h"
      ]
    }

  files: [
    "*.cpp",
    "*.h"
    ]
  }
