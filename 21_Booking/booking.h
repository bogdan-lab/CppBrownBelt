#pragma once

namespace RAII{

template <typename Provider>
class Booking{
private:
    Provider* el;
    //int cc_;
public:
    Booking() = default;
    Booking(Provider* given_el, int cc): el(given_el){}
    Booking(const Booking<Provider>& other) = delete;
    Booking<Provider>& operator=(const Booking<Provider>& other) = delete;

    Booking& operator=(Booking&& other){
        if(this!=&other){
            this->~Booking();
            this->el = other.Release();
        }
        return *this;
    }

    Booking(Booking<Provider>&& other){
        this->el = other.Release();
    }


    const Provider& GetProvider() const{
        return *el;
    }

    Provider* Release() {
        Provider* tmp = el;
        el = nullptr;
        return tmp;
    }

    ~Booking(){
        if(el){
            el->CancelOrComplete(*this);
        }
    }
};
}
