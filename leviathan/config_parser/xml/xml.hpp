// https://learn.microsoft.com/zh-cn/previous-versions/dotnet/netframework-4.0/ms256177(v=vs.100)

/*
    Elements: 
        - <elementName att1Name="att1Value" att2Name="att2Value"...> </elementName>
        - <elementName att1Name="att1Value" att2Name="att2Value".../>
    Prolog:
        - <?xml version="1.0" encoding="UTF-8"?>
    Comment:
        - <!--- <test pattern="SECAM" /><test pattern="NTSC" /> -->
    Character Reference:
        - &lt '<'
        - &gt '>'
        - &amp '&'
        - &apos '''
        - &quot '"'
    CDATA:
        - <![CDATA[An in-depth look at creating applications with XML, using <, >,]]>
    Attribute:
        - <myElement contraction="isn't" />
        - <myElement question='They asked "Why?"' />
*/  

#pragma once

namespace leviathan::config::xml
{
    class document;
    class attribute;
    class elements;
} // namespace leviathan::config::xml

