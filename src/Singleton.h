#pragma once

template <class T>
class Singleton
{
public:
	static T* instance()
	{
		static CGuard g;
		if (!g_instance)
			g_instance = new T();
		return g_instance;
	}

protected: // constructors
	Singleton() = default;
	virtual ~Singleton() = default;
private: // copy constructor
	Singleton(const Singleton&) = delete;

protected:
	static T* g_instance;

private:
	class CGuard
	{
	public:
		~CGuard()
		{
			if (NULL != T::g_instance)
			{
				delete T::g_instance;
				T::g_instance = nullptr;
			}
		}
	};
};
