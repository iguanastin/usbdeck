template <class T> class LateArray {
  public:
    bool initialized = false;
    int len = 0;
    T** arr;

    T& operator[](int i) {
      return *arr[i];
    }
    bool init(int length) {
      if (initialized || length < 1) return false;

      len = length;
      initialized = true;
      arr = new T*[len];
      
      return true;
    }
};

// Check if two char arrays exactly match for a specified length
bool strMatch(const char* str1, const char* str2, const int len);

// Splits a 4 byte int into the provided buffer
void splitIntToBytes(const int number, char* bytes);

// Joins the provided 4 bytes into a single int
int joinBytesToInt(const char* bytes);