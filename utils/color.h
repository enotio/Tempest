#ifndef COLOR_H
#define COLOR_H

namespace Tempest{


	//! Цвет, rgba, [0..1], одинарная точность.
	class Color{
		public:
		//! Тип, используемый для хранения канала.
		typedef double ChanelType;

			//! Констуктор. Эквивалентно Color(1.0).
			Color();
			//! Тоже конструктор. Всем каналам будет задано значение rgba.
			Color( ChanelType rgba );
			//! Конструктор.
			Color( ChanelType r, ChanelType g,
             ChanelType b, ChanelType a = 1.0 );


			//! Задание всех каналов явно.
			void set(ChanelType r,
							 ChanelType g,
							 ChanelType b,
							 ChanelType a);
			//! Перегруженная функция, введена для удобства.
			void set( ChanelType rgba );

			//! Получить 4х элементный массив,
			//! элементы которого являются значениями каналов.
			const ChanelType * data() const;

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
			ChanelType r() const;
			//! Канал green.
			ChanelType g() const;
			//! Канал blue.
			ChanelType b() const;
			//! alpha канал.
			ChanelType a() const;
		private:
			ChanelType cdata[4];
	};

}

#endif // COLOR_H
