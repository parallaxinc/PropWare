#ifndef LIBPROPELLER_STREAMINTERFACE_H_
#define LIBPROPELLER_STREAMINTERFACE_H_

template<class Derived>
struct InputStream{
    int Get(const int timeout = -1){
        return static_cast<Derived*>(this)->Get(timeout);
    }
    
    int Get(char * const buffer, const int length, const int timeout = -1){
        return static_cast<Derived*>(this)->Get(buffer, length, timeout);
    }
    
    int Get(char * const buffer, const char terminator = '\n'){
        return static_cast<Derived*>(this)->Get(buffer, terminator);
    }
    
    
};

template<class Derived>
struct OutputStream{
    void Put(const char character){
        static_cast<Derived*>(this)->Put(character);
    }
    
    int Put(const char * buffer_ptr){
        return static_cast<Derived*>(this)->Put(buffer_ptr);
    }
    
    int Put(const char * buffer_ptr, const int count){
        return static_cast<Derived*>(this)->Put(buffer_ptr, count);
    }
};

#endif // LIBPROPELLER_STREAMINTERFACE_H_

