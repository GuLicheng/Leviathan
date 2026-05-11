#pragma once

/*
    How can we parse a json-string to some structure?

    Firstly, we need to parse the json-string to some intermediate representation, 
    which can be a map of string to string, or a vector of string, etc. 
    Then we can use the intermediate representation to construct the final structure.

    For constructing the final structure, we can use some annotations to specify how 
    to map the intermediate representation to the final structure. In that case, we
    must be able to get the annotation of a field, and then use the annotation to get the
    mapping function, and then use the mapping function to get the final value of the field.

    So we need some annotations to specify the mapping function for each field. 
    For example, we can have a shortname annotation, which specifies that the field should 
    be mapped to a short name, and a longname annotation, which specifies that the 
    field should be mapped to a long name, etc.

    Follows are our annotation:

    - rename: specify the name of the field.
    
    - ignore: format or scan maybe ignored some fields, this annotation is used to mark those fields.
    
    - value: specify the default value of the field, this is useful when the field is missing.
    
    - parser: for given type T and input type I, call std::invoke(parser, instanceof I) and cast result to T. 

    TODO:
    
    - required: mark a field as required, if the field is missing, throw an error.
    
    - choice: for given type T and a list of options, try to cast input to each option, 
    if any option is successful, return the result, otherwise throw an error.

*/

#pragma once

#include <string>
#include <functional>
#include <meta>

namespace cpp::refl
{


template <typename F>
struct simple_function
{
    F function;

    constexpr explicit simple_function(F function) : function(std::move(function)) {}

    template <typename Self, typename... Args>
    constexpr std::string operator()(this Self&& self, Args&&... args)
    {
        return std::invoke(((Self&&)self).function, (Args&&)args...);
    }
};

struct annotation { }; 

struct debug_annotation : annotation { };





















}


