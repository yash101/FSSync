#ifndef SMARTPTR_H
#define SMARTPTR_H
template<typename T>
class SmartPointer
{
private:
  T* data;
  unsigned long long n;
public:
  inline SmartPointer()
  {
    data = nullptr;
    n = 0;
  }
  inline SmartPointer(T* ptr)
  {
    data = ptr;
    n = 1;
  }
  inline SmartPointer(T* ptr, decltype(n) num)
  {
    data = ptr;
    n = num;
  }

  inline void set(T* ptr)
  {
    data = ptr;
    n = 1;
  }

  inline void set(T* ptr, decltype(n) num)
  {
    data = ptr;
    n = num;
  }

  inline T* get()
  {
    return data;
  }

  inline decltype(n) size()
  {
    return n;
  }

  inline ~SmartPointer()
  {
    if(data)
    {
      if(n == 1)
        delete data;
      else
        delete[] data;
    }
  }
};

#endif // SMARTPTR_H
