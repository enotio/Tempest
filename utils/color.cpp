#include "color.h"

using namespace Tempest;

Color::Color(){
		set( 1.0 );
		}

Color::Color( ChanelType c ){
		set( c );
		}

Color::Color(	ChanelType r, ChanelType g,
							ChanelType b, ChanelType a){
		set(r, g, b, a);
		}

void Color::set( ChanelType r,
								 ChanelType g,
								 ChanelType b,
								 ChanelType a){
		cdata[0] = r;
		cdata[1] = g;
		cdata[2] = b;
		cdata[3] = a;
		}

void Color::set( ChanelType rgba ){
		for(int i=0; i<4; ++i)
			cdata[i] = rgba;
		}

const Color::ChanelType * Color::data() const{
		return cdata;
		}

Color& Color::operator = ( const Color & other){
		for(int i=0; i<4; ++i)
			cdata[i] = other.cdata[i];

		return *this;
		}

Color Color::operator + ( const Color & other){
		Color c;
		for(int i=0; i<4; ++i)
			c.cdata[i] = cdata[i]+other.cdata[i];

		return c;
		}

Color Color::operator - ( const Color & other){
		Color c;
		for(int i=0; i<4; ++i)
			c.cdata[i] = cdata[i]-other.cdata[i];

		return c;
		}

void Color::operator += ( const Color & other){
		for(int i=0; i<4; ++i)
			cdata[i] += other.cdata[i];

		}

void Color::operator -= ( const Color & other){
		for(int i=0; i<4; ++i)
			cdata[i] -= other.cdata[i];

		}

Color::ChanelType Color::r() const{
		return data()[0];
		}

Color::ChanelType Color::g() const{
		return data()[1];
		}

Color::ChanelType Color::b() const{
		return data()[2];
		}

Color::ChanelType Color::a() const{
		return data()[3];
		}
