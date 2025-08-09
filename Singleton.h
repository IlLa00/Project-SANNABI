#pragma once

template<typename T>
class Singleton
{
public:
	static T* GetInstance()
	{
		return &_instance;
	}
	static void DestroyInstance()
	{
		//if (_instance != nullptr)
		{
			//delete _instance;
			//_instance = nullptr;
		}
	}

protected:
	Singleton() = default;
	virtual ~Singleton() = default;

	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
	Singleton(Singleton&&) = delete;
	Singleton& operator=(Singleton&&) = delete;

private:
	static T _instance;	
};

template<typename T>
T Singleton<T>::_instance;