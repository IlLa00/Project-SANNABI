#pragma once

// 화면 해상도
constexpr int GWinSizeX = 1920;
constexpr int GWinSizeY = 1080;

constexpr int mapSizeX = 5300;
constexpr int mapSizeY = 3150;

// 엔씨에 있을때 사용하던 방식 & 언리얼도 이렇게 씁니다.
using int8 = char;					// 정수 저장하는데 1bit 짜리
using int16 = short;
using int32 = int;
using int64 = long long;		// 매번 long long 쓰기 귀찮아서 int64
using uint8 = unsigned char;
using uint16 = unsigned short;
using uint32 = unsigned int;
using uint64 = unsigned long long;

float RadianToDegree(float radian);
float DegreeToRadian(float degree);

// 언리얼에서 가져옴.
#define SMALL_NUMBER			(1.e-8f)

struct PairHash
{
	template <class T1, class T2>
	size_t operator()(const std::pair<T1, T2>& p) const
	{
		auto h1 = std::hash<T1>{}(p.first);
		auto h2 = std::hash<T2>{}(p.second);

		// 두 해시값을 조합
		return h1 ^ (h2 << 1);
	}
};

// vector 구현 ((방향+크기)를 나타내는 벡터)
struct Vector
{
	// 좌표
	float x = 0;
	float y = 0;

	Vector() : x(0), y(0) {}
	Vector(float x, float y) : x(x), y(y) {}
	Vector(POINT pt) : x(pt.x), y(pt.y) {}

	//Vector p1;
	//Vector p2;
	//Vector p3 = Vector(p1.x + p2.x, p1.y + p2.y);
	//Vector p3 = p1 + p2;
	//p1 += p2;	// 이런 덧셈도 할수 있을지도
	//
	//int a = 10;
	//a += 1;

	// 사칙연산 정의
	// +, - , *, / 
	Vector operator+(const Vector& other)
	{
		Vector ret;
		ret.x = x + other.x;
		ret.y = y + other.y;
		return ret;
	}
	Vector operator-(const Vector& other)
	{
		Vector ret;
		ret.x = x - other.x;
		ret.y = y - other.y;
		return ret;
	}
	Vector operator*(float ratio)
	{
		Vector ret;
		ret.x = x * ratio;
		ret.y = y * ratio;
		return ret;
	}

	void operator+=(const Vector& other)
	{
		x += other.x;
		y += other.y;
	}
	void operator-=(const Vector& other)
	{
		x -= other.x;
		y -= other.y;
	}
	void operator*=(float ratio)
	{
		x *= ratio;
		y *= ratio;
	}

	// 벡터를 나눗셈할일이 많지가 않다...
	// 연산자 안만들어도 쓸일이 거의없을듯.


	// 벡터의 길이 구하기
	// 엔진 살펴보면, Legnth, LengthSquared (벡터는 보통 길이를 리턴하는 함수가 2종류)
	// sqrt(루트) 연산은 생각보다 비싸기 때문에, 꼭 진짜 길이가 필요한 경우가 아니면,
	// LengthSquared (제곱 길이) 버전을 사용하는게 더 좋다.
	float Length()
	{
		return ::sqrt(x * x + y * y);
	}

	// 길이의 제곱 버전
	float LengthSquared()
	{
		return (x * x + y * y);
	}

	// 이것도 게임만들다가. 다양한 버전이 필요하면 여기에 재정의 하면 되요.
	void Normalize()
	{
		// 스스로 노멀라이즈 하는 버전만 만들거지만.
		// 스스로 노멀라이즈 하지 않고, 리턴하는 버전을 추가로 만들수도 있어요.

		// 벡터의 크기를 구해서, 크기만큼 나누기 해주면 된다.
		float length = Length();

		// 언리얼 엔진은 풀 오픈소스. 언리얼 참고해서 공부해도 좋습니다.
		//length 가 0이거나 엄청 작을수 있다
		if (length < SMALL_NUMBER)
			return;	// 굳이 정규화안하고 리턴

		// 정규화.
		x /= length;
		y /= length;
	}

	Vector Normalized()
	{
		Vector normalizedVector = *this;

		float length = Length();

		if (length < SMALL_NUMBER)
			return Vector();

		normalizedVector.x /= length;
		normalizedVector.y /= length;

		return normalizedVector;
	}

	// 내적은 Dot 
	float Dot(const Vector& other)
	{
		return x * other.x + y * other.y;
	}

	// radian 각도만큼 회전시키자.
	Vector Rotate(float radian)
	{
		float cosA = ::cosf(radian);
		float sinA = ::sinf(radian);

		return Vector(x * cosA - y * sinA, x * sinA + y * cosA);
	}

	// 외적은 Cross
	float Cross(const Vector& other)
	{
		//2D 라서, 스칼라값만 반환을 한다. 
		// 법선 벡터가 필요한게 아니고, 부호를 판단하는 스칼라값이 필요하다.
		return x * other.y - y * other.x;
	}
};

enum class ECollisionChannel
{
	WorldStatic, // 정적인 지형
	WorldDynamic, // 동적인 플랫폼
	DeathTile,
	Character, // 캐릭터
	CharacterInvincible, // 캐릭터 무적
	Projectile, // 발사체
	Perception, // 감지영역

	Max
};

const int TileSize = 52;
const int OriginTileSize = 52;	// 원본 타일 사이즈

const int GridWidth = 120;
const int GridHeight = 80;

const int TileMapWidth = 4;
const int TileMapHeight = 4;

const int BuildingMapWidth = 4;
const int BuildingMapHeight = 4;

constexpr const wchar_t* fontPath = L"C:\\KHJ\\SVN\\Project-SANNABI\\Font";
constexpr const wchar_t* fontName = L"NotoSansTC-Regular";

extern HFONT hFont;
