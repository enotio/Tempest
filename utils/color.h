#ifndef COLOR_H
#define COLOR_H

namespace Tempest{

	//! Цвет, rgba, [0..1], одинарная точность.
	class Color{
    public:
			//! Констуктор. Эквивалентно Color(1.0).
			Color();

			//! Тоже конструктор. Всем каналам будет задано значение rgba.
      Color( float rgba );

			//! Конструктор.
      Color( float r, float g,
             float b, float a = 1.0 );


			//! Задание всех каналов явно.
      void set(float r,
               float g,
               float b,
               float a);
			//! Перегруженная функция, введена для удобства.
      void set( float rgba );

			//! Получить 4х элементный массив,
			//! элементы которого являются значениями каналов.
      const float * data() const;

			//! Присваивание.
			Color& operator = ( const Color & other);

			//! Сложение. Результат не будет обрубаться по диапозону.
			Color  operator + ( const Color & other);
			//! Вычитание. Результат не будет обрубаться по диапозону.
			Color  operator - ( const Color & other);

			//! Сложение. Результат не будет обрубаться по диапозону.
			void operator += ( const Color & other);
			//! Вычитание. Результат не будет обрубаться по диапозону.
			void operator -= ( const Color & other);

			//! Канал red.
      float r() const;
			//! Канал green.
      float g() const;
			//! Канал blue.
      float b() const;
			//! alpha канал.
      float a() const;

      bool operator == ( const Color & other ) const {
        return cdata[0]==other.cdata[0] &&
               cdata[1]==other.cdata[1] &&
               cdata[2]==other.cdata[2] &&
               cdata[3]==other.cdata[3];
        }

      bool operator !=( const Color & other ) const {
        return !(*this==other);
        }
		private:
      float cdata[4];
	};

}

#endif // COLOR_H
