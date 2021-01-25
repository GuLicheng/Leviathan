#include "INI.hpp"


namespace leviathan::INI
{



}  // namespace leviathan

int main()
{
    using namespace leviathan::INI;
    INI_handler ini;
    ini.load("./rulesmo.ini");
    ini.show();
    std::cout << "lines : " << ini.lines << std::endl;
    std::cout << "Test successfully\n";
}



/**

 * !!! 我们的目的是能够正确读取红色警戒系列中的ini配置文件

 * 说明：ini配置文件读写

 * 1、支持;注释符号。// 非必须不支持 # 注释符号

 * 2、不支持带引号'或"成对匹配的字符串，否则无法正确读取相关词条，比如 name = stalin's fist

 * 3、如果无section，将读取不到任何信息，但section可以为空。

 * 4、不支持10、16、8进制数，0x开头为16进制数，0开头为8进制，但是我们日后可能会为此提供专门的接口。

 * 5、支持section、entry或=号前后带空格。

 * 6、按行读取，不接受换行。如何您有更高的要求，请使用json文件。

 * 7、区分section、key大小写。

 * 8、不支持任何修改ini操作，因为我们只是读取文件，配置文件建议在源文件上手动修改，实际上大家在制作mod的时候都是这么做的

 * 9、读取文件的时候不会对section和entry部分做任何检查， 但是额外设置了一个接口会对读取后的结果进行检查，如何需要的话可以使用

 */

