#ifndef ITEXTURE_H
#define ITEXTURE_H

namespace Tempest{

  //! Интерфейс класса текстуры.
  class AbstractTexture {
    public:
      //typedef unsigned int FilterType;
      //! Способы фильтрации текстуры.
      struct FilterType{
        //! Способы фильтрации текстуры.
        enum Type{
          //! ближайшая фильтрация
          Nearest,
          //! линейная фильтрация
          Linear,

          //! Count
          Count
          };
        };

      //! Формат, в которром текстура будет хранится на GPU.
      //! Рекомедую форматы RGBA_DXT5, RGB_DXT1 - для дифузных,
      //! нормальных, спекуляр текстур,
      //! RGBA, RGB - для GUI и HUD текстур.
      struct Format{
        //! Тип формата.
        enum Type{
          Luminance,
          Luminance4,
          Luminance8,
          Luminance16,

          RGB,
          RGB4,
          RGB5,
          RGB10,
          RGB12,
          RGB16,

          RGBA,
          RGBA8,
          RGB10_A2,
          RGBA12,
          RGBA16,


          RGB_DXT1,
          RGBA_DXT1,
          RGBA_DXT3,
          RGBA_DXT5,

          Depth16,
          Depth24,
          Depth32,

          RG16,

          Count
          };
        };

      //! Формат, в которром текстура хранится в ОЗУ.
      //! При загрузке тексели будут преобразованны к Tempest::ITexture::Format.
      struct InputFormat{
        //! Формат, в которром текстура хранится в ОЗУ.
        enum Type{
          Luminance8,
          RGB8,
          RGBA8,
          Depth,
          Count
          };
        };

      struct ClampMode{
        enum Type{
          Clamp, //GL_CLAMP,
          ClampToBorder, //GL_CLAMP_TO_BORDER,
          ClampToEdge, //GL_CLAMP_TO_EDGE,
          MirroredRepeat, //GL_MIRRORED_REPEAT,
          Repeat, //GL_REPEAT
          Count
          };
        };

      virtual ~AbstractTexture(){}

    };

  }

#endif // ITEXTURE_H
