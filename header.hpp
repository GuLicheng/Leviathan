#pragma once
#include <meta>

namespace meta = std::meta;

//map
consteval std::string info_to_string(meta::info r) {
    auto sv = meta::display_string_of(r);
    return std::string(sv.data(), sv.size());
}

consteval bool is_info_a_value(meta::info r){
    return meta::is_value(r) || meta::is_object(r);
}

template <meta::info r>
concept is_proper_sotrage_type =
    meta::is_class_template(r) &&meta::can_substitute(r,{meta::reflect_constant(^^void)})
    && !meta::is_complete_type(meta::substitute(r,{meta::reflect_constant(^^void)}));

template <meta::info storageR>
requires is_proper_sotrage_type<storageR>
struct consteval_map {
    private:
        consteval_map() = delete;
    
    template <auto v>
    struct value_tag {
        static constexpr auto value = v;
    };
    public:
    static consteval bool check_refl(meta::info k) {
        return meta::is_complete_type(meta::substitute(storageR, {meta::reflect_constant(k)}));
    }

    template <meta::info k, meta::info v>
    static consteval void put_refl() {
        if constexpr (!check_refl(k)) {
            auto valueRefl = ^^value_tag<v>;
            //if(!meta::is_complete_type(meta::substitute(storageR, {meta::reflect_constant(k)})))
            meta::define_aggregate(meta::substitute(storageR, {meta::reflect_constant(k)}),
                                   {meta::data_member_spec(valueRefl, {.name = "value"})});
        } else {
            constexpr auto value = get_refl<k>();
            static_assert(false, "key: \"" + info_to_string(k) +
                                     "\" all ready exists with value \"" + info_to_string(value) +
                                     "\", not \"" + info_to_string(v) + "\"");
        }
    }
    struct map_error_key_doesnt_exist {};
    template <meta::info k>
    static consteval meta::info get_refl() {
        if constexpr (!check_refl(k)) {
            static_assert(false, "no key: \"" + info_to_string(k) + "\"");
            return ^^map_error_key_doesnt_exist;
        } else {
            using strgT = typename[:meta::substitute(storageR, {meta::reflect_constant(k)}):];
            return decltype(strgT::value)::value;
        }
    }
    private:
    template<meta::info k>
    static consteval auto get_value(){
        constexpr meta::info refl = get_refl<k>();
        if constexpr(is_info_a_value(refl)){
            return [:refl:];
        }else{
            static_assert(false,"the reflection of \""+info_to_string(refl)+"\" is not a value or object");
            return;
        }
    }

    template<meta::info k>
    static consteval auto get_type() -> typename[:get_refl<k>():]{
        auto refl = get_refl<k>();
        static_assert(meta::is_type(refl),"reflection \""+info_to_string(refl)+"\" not a type");
    }
    public:
    template<typename k>
    static consteval bool check(){
        return check_refl(^^k);
    }

    template<auto k>
    static consteval bool check(){
        return check_refl(meta::reflect_constant(k));
    }

    template<typename k>
    static consteval bool check_is_type(){
        if constexpr(check<k>()){
            return meta::is_type(get_refl<^^k>());
        }else{
            return false;
        }
    }

    template<typename k>
    static consteval bool check_is_value(){
        if constexpr(check<k>()){
            return is_info_a_value(get_refl<^^k>());
        }else{
            return false;
        }
    }

    template<auto k>
    static consteval bool check_is_type(){
        if constexpr(check<k>()){
            return meta::is_type(get_refl<meta::reflect_constant(k)>());
        }else{
            return false;
        }
    }

    template<auto k>
    static consteval bool check_is_value(){
        if constexpr(check<k>()){
            return is_info_a_value(get_refl<meta::reflect_constant(k)>());
        }else{
            return false;
        }
    }

    template<typename k, typename v>
    static consteval void put(){
        put_refl<^^k,^^v>();
    }

    template<typename k, auto v>
    static consteval void put(){
        put_refl<^^k,meta::reflect_constant(v)>();
    }

    template<auto k, typename v>
    static consteval void put(){
        put_refl<meta::reflect_constant(k),^^v>();
    }

    template<auto k, auto v>
    static consteval void put(){
        put_refl<meta::reflect_constant(k),meta::reflect_constant(v)>();
    }

    template<typename k>
    consteval meta::info get(){
        return get_refl<^^k>();
    }

    template<auto k>
    consteval meta::info get(){
        return get_refl<meta::reflect_constant(k)>();
    }

    template<typename k>
    using getT_t = decltype(get_type<^^k>());

    template<typename k>
    static constexpr auto getT_v = get_value<^^k>(); 

    template<auto k>
    using getV_t = decltype(get_type<meta::reflect_constant(k)>());

    template<auto k>
    static constexpr auto getV_v = get_value<meta::reflect_constant(k)>();

};

// mutable


template<meta::info storageR, std::size_t search_hint = 100, meta::info key = ^^void>
requires (search_hint >1)
struct consteval_mutable{
    private:
    consteval_mutable() =delete;
    using storage = consteval_map<storageR>;

    static consteval meta::info get_sub_refl(std::size_t index){
        return meta::reflect_constant(std::pair<meta::info,std::size_t>{key,index}); 
    }
    public:
    static consteval std::size_t get_last_index(){
        std::size_t r = search_hint;
        while(storage::check_refl(get_sub_refl(r))){
            r*=search_hint;
        }
        std::size_t l =r/search_hint-1;
        while(l<r){
            std::size_t mid = (l+r)/2;
            if(storage::check_refl(get_sub_refl(mid))){
                l = mid+1;
            }else{
                r = mid;
            }
        }
        return l-1;
    }

    template<meta::info v, std::size_t index = get_last_index()+1>
    static consteval void put_refl(){
        storage::template put_refl<get_sub_refl(index),v>();
    }

    template<auto v,std::size_t index = get_last_index()+1>
    static consteval void put(){
        put_refl<meta::reflect_constant(v),index>();
    }

    template<typename v,std::size_t index = get_last_index()+1>
    static consteval void put(){
        put_refl<^^v,index>();
    }

    struct consteval_mutable_error_its_empty;
    template<std::size_t index = get_last_index()>
    static consteval meta::info get_refl(){
        if constexpr(index != -1){
            return storage::template get_refl<get_sub_refl(index)>();
        }else{
            static_assert(false,"the mutable with storage reflection \""+info_to_string(storageR)+"\" is empty");
            return ^^consteval_mutable_error_its_empty;
        }
    }
    private:
    template<std::size_t index>
    static consteval auto get_v_impl(){
        constexpr auto refl = get_refl<index>();
        if constexpr(is_info_a_value(refl)){
            return [:refl:];
        }else{
            static_assert(false,"the reflection \""+info_to_string(refl)+"\" is not a value or object");
            return;
        }
    }

    template<std::size_t index>
    static consteval auto get_type_impl()->typename[:get_refl<index>():]{
        auto refl = get_refl<index>();
        static_assert(meta::is_type(refl),"the reflection \""+info_to_string(refl)+"\" is not a type");
    }
    public:
    template<std::size_t index = get_last_index()>
    static constexpr auto get_v = get_v_impl<index>();

    template<std::size_t index = get_last_index()>
    using get_t = decltype(get_type_impl<index>());

    static consteval bool check(){
        return get_last_index()!=static_cast<std::size_t>(-1);
    }
    template<std::size_t index = get_last_index()>
    static consteval bool check_is_value(){
        if constexpr(check()){
            return is_info_a_value(get_refl<index>());
        }else{ 
            return false;
        }
    }
    template<std::size_t index = get_last_index()>
    static consteval bool check_is_type(){
        if constexpr(check()){
            return meta::is_type(get_refl<index>());
        }else{ 
            return false;
        }
    }

};

// mut map


template<meta::info storageR, std::size_t mutable_hint = 100>
struct consteval_mutable_map{
    private:
        consteval_mutable_map()=delete;
    public:

    template<auto key>
    using get_mutable_typeV_t = consteval_mutable<storageR,mutable_hint,meta::reflect_constant(key)>;

    template<typename key>
    using get_mutable_typeT_t = consteval_mutable<storageR,mutable_hint,^^key>;

    
    template<auto key>
    static consteval bool check(){
        return get_mutable_typeV_t<key>::check();
    }

    template<typename key>
    static consteval bool check(){
        return get_mutable_typeT_t<key>::check();
    }

    template<auto key>
    static consteval bool check_is_value(){
         if constexpr(check<key>()){
            return get_mutable_typeV_t<key>::check_is_value();
         }else{
            return false;
         }
    }
    
    template<auto key>
    static consteval bool check_is_type(){
         if constexpr(check<key>()){
            return get_mutable_typeV_t<key>::check_is_type();
         }else{
            return false;
         }
    }

    template<typename key>
    static consteval bool check_is_value(){
         if constexpr(check<key>()){
            return get_mutable_typeT_t<key>::check_is_type();
         }else{
            return false;
         }
    }

    template<typename key>
    static consteval bool check_is_type(){
         if constexpr(check<key>()){
            return get_mutable_typeT_t<key>::check_is_type();
         }else{
            return false;
         }
    }


    template<auto key, auto value, std::size_t index = get_mutable_typeV_t<key>::get_last_index()>
    static consteval void put(){
        get_mutable_typeV_t<key>::template put<value>();
    }

    template<auto key, typename value, std::size_t index = get_mutable_typeV_t<key>::get_last_index()>
    static consteval void put(){
        get_mutable_typeV_t<key>::template put<value>();
    }

    template<typename key, auto value, std::size_t index = get_mutable_typeT_t<key>::get_last_index()>
    static consteval void put(){
        get_mutable_typeT_t<key>::template put<value>();
    }

    template<typename key, typename value, std::size_t index = get_mutable_typeT_t<key>::get_last_index()>
    static consteval void put(){
        get_mutable_typeT_t<key>::template put<value>();
    }


    template<auto key, std::size_t index = get_mutable_typeV_t<key>::get_last_index()>
    static constexpr auto getV_v = get_mutable_typeV_t<key>::template get_v<index>;

    template<typename key, std::size_t index = get_mutable_typeT_t<key>::get_last_index()>
    static constexpr auto getT_v = get_mutable_typeT_t<key>::template get_v<index>;

    template<auto key, std::size_t index = get_mutable_typeV_t<key>::get_last_index()>
    using getV_t = get_mutable_typeV_t<key>::template get_t<index>;

    template<typename key, std::size_t index = get_mutable_typeT_t<key>::get_last_index()>
    using getT_t = get_mutable_typeT_t<key>::template get_t<index>;

};

// counter

template<meta::info storageR, std::size_t hint = 100>
struct consteval_counter{

    private:
    static consteval meta::info substitute(std::size_t index){
        return meta::substitute(storageR, { std::meta::reflect_constant(index) });
    }
    consteval_counter() = delete;
    public:

    static consteval std::size_t get(){
        std::size_t r = hint;
        while(meta::is_complete_type(substitute(r))){
            r*=hint;
        }
        std::size_t l =r/hint-1;
        while(l<r){
            std::size_t mid = (l+r)/2;
            if(meta::is_complete_type(substitute(mid))){
                l = mid+1;
            }else{
                r = mid;
            }
        }
        return l;
    }

    static consteval void increment(std::size_t n = 1){
        std::size_t offset = get();
            for(int i=0;i<n;i++){
                meta::define_aggregate(substitute(offset+i),{});
            }
    }
};


consteval std::size_t processKey(std::size_t key){
    return key | ( 1<<31);
}
// any counter based rng can be used
// https://arxiv.org/pdf/2004.06278 is used as an example
consteval std::size_t square_CBRNG(std::size_t ctr, std::size_t key){
    key = processKey(key);
    std::size_t t,x,y,z;
    y = x = ctr*key;
    z = y+key;
    x = x*x + y; 
    x = (x>>32) | (x<<32);
    x = x*x+z;
    x = (x>>32) | (x<<32);
    x = x*x +y;
    x = (x>>32) | (x<<32);
    t = x = x*x +z;
    x = (x>>32) | (x<<32);
    return t ^ ((x*x +y) >> 32);
}

template<typename T>
concept is_not_void = !std::is_same_v<T,void>;


template<typename T, typename B>
concept is_InRange_compatible = requires(T ret, B v){
    {ret%(v-v)+v}->is_not_void;
};

template<typename T>
concept proper_cbrng_lambda = requires(std::size_t a,std::size_t b){
    {T{}(a,b)} -> is_not_void;
};

using defualt_cbrng_lambda = decltype([](std::size_t ctr, std::size_t key){return square_CBRNG(ctr,key);});

template<meta::info storageR, std::size_t keyV, proper_cbrng_lambda F = defualt_cbrng_lambda>
struct consteval_rng{
    private:
    using counter = consteval_counter<storageR>;
    consteval_rng()=delete;
    public:
    static constexpr std::size_t key = keyV;

    static consteval void next(){
        counter::increment();
    }

    static consteval auto get(){
        return F{}(counter::get(),key);
    }
    template<typename T>
    static consteval auto getInRange(T min, T max){
        using retT = decltype(F{}(0,0));
        if constexpr(is_InRange_compatible<retT,T>){
            std::size_t range = max-min;
            return get()%range + min;
        }else{
            static_assert(false,"getInRange cannot be used as the provided lambda return type R and the template type T cannot be used in the expresion: R%(T-T)+T");
            return;
        }
    }

};  

// enum


template <std::size_t N>
    struct universal_enum_fixed_string {
        char data[N] = {};

        consteval universal_enum_fixed_string(const char (&str)[N]) {
            for (int i = 0; i < N; i++) {
                data[i] = str[i];
            }
        }

        constexpr char operator[](std::size_t i) const { return data[i]; }

        constexpr const char* to_cstr() const { return data; }
    };

template <meta::info storageR>
struct universal_enum {
    private:
    universal_enum() = delete;
    using map = consteval_map<storageR>;
    using counter = consteval_counter<storageR>;
    public:
    template<universal_enum_fixed_string str>
    static consteval bool check(){
        return map::template check<str>();
    }

    template<universal_enum_fixed_string str>
    static consteval bool check_is_value(){
        return map::template check_is_value<str>();
    }

    template<universal_enum_fixed_string str>
    static consteval bool check_is_type(){
        return map::template check_is_type<str>();
    }

    template<universal_enum_fixed_string str>
    static consteval void put(){
        map::template put<str,counter::get()>();
        counter::increment();
    }

    template<universal_enum_fixed_string str, auto v>
    static consteval void put(){
        map::template put<str,v>();
    }

    template<universal_enum_fixed_string str, typename T>
    static consteval void put(){
        map::template put<str,T>();
    }

    template<universal_enum_fixed_string str>
    using get_t = map::template getV_t<str>;

    template<universal_enum_fixed_string str>
    static constexpr auto get_v = map::template getV_v<str>;
};
