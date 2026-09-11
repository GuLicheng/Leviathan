#include "all.hpp"
#include <print>
#include <leviathan/extc++/all.hpp>

using Context = winnow::stream<winnow::context_error>;

class InIParser
{
public:

    static auto Parse(Context& ctx) {

        auto comment_consumer = winnow::combinator::delimited(
            winnow::combinator::preceded(
                winnow::ascii::multispace0,  // consume leading whitespace
                winnow::token::literal(";")  // comment indicator
            ),
            winnow::ascii::till_line_ending,
            winnow::ascii::line_ending
        );

        auto left_parenthesis = winnow::combinator::delimited(
            winnow::ascii::multispace0,
            winnow::token::literal("["),
            winnow::ascii::multispace0
        );

        auto right_parenthesis = winnow::combinator::delimited(
            winnow::ascii::multispace0,
            winnow::token::literal("]"),
            winnow::combinator::terminated(
                winnow::ascii::multispace0,
                winnow::combinator::opt(comment_consumer)
            )
        );

        auto identifier = winnow::token::take_while([](char c) {
            return std::isalnum(c) || c == '.';
        }, 1);

        auto section_parser = winnow::combinator::delimited(
            left_parenthesis,
            identifier,
            right_parenthesis
        );

        // key = value

        auto key_parser = winnow::combinator::delimited(
            winnow::ascii::multispace0,
            identifier,
            winnow::ascii::multispace0
        );

        auto value_parser = winnow::combinator::delimited(
            winnow::ascii::multispace0,
            winnow::token::take_while([](char c) {
                return ValidCharacters.contains(c);
            }, 1),
            winnow::combinator::terminated(
                winnow::ascii::multispace0,
                winnow::combinator::opt(comment_consumer)
            )
        );

        auto entry_parser = winnow::combinator::repeat<std::vector>(
            winnow::combinator::separated_pair(
                key_parser,
                winnow::token::literal("="),
                value_parser
            )
        );

        auto lines_parser = winnow::combinator::repeat<std::vector>(
            winnow::combinator::sequence(
                section_parser,
                winnow::combinator::opt(entry_parser)
            )
        );

        auto parser = winnow::combinator::preceded(
            winnow::combinator::repeat<std::vector>(comment_consumer),
            lines_parser
        );

        auto result = parser(ctx);

        if (!result)
        {
            throw std::runtime_error("Parsing failed");
        }

        using ResultDictionary = std::unordered_map<
            std::string, 
            std::unordered_map<std::string, std::string>    
        >;

        auto temp = result.value() | cpp::views::pair_transform(
            [] (auto ctx) static { return std::string(ctx.begin(), ctx.end()); },
            [] (auto optEntries) static { 
                return *optEntries | 
                       cpp::views::pair_transform(&InIParser::trim_str, &InIParser::trim_str) |
                       std::ranges::to<std::unordered_map<std::string, std::string>>();
            }
        ) | std::ranges::to<ResultDictionary>();

        return temp;
    }

    static std::string trim_str(Context sv)
    {
        return std::string(cpp::string::trim(sv.to_string_view()));
    }

    static constexpr std::string_view ValidCharacters = 
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789"
            "`~!@#$%^&*()-_=+[{]}\\|:'\",<.>/? ";

};

constexpr const char* ini = R"(

[BORIS]
Name=Boris
UIName=Name:Boris
Image=BORIS
Category=Soldier
Primary=AKM           ;普通级主武器 AK步枪
Secondary=FLARE       ;副武器：激光指示器呼叫米格
Strength=200          ;生命值
Armor=plate           ;装甲 plate=防弹衣
TechLevel=9
Sight=9               ;视野
Speed=5               ;移动速度
Cost=1500             ;造价
Soylent=750
BuildLimit=1          ;英雄限制：只能造1个
Owner=Russians,Confederation,Africans,Arabs
RequiredHouses=Russians
Requires=BARRACK,TECH;建造条件：兵营+作战实验室
PipScale=1
Points=25
ThreatPosed=25
; 升级能力
VeteranAbilities=STRONGER,FIREPOWER,ROF,SIGHT,FASTER
EliteAbilities=SELF_HEAL,STRONGER,FIREPOWER,ROF
; 免疫属性（重点）
ImmuneToPsionics=yes        ;免疫心灵控制
ImmuneToPsionicWeapons=yes  ;免疫心灵震荡
ImmuneToVeins=yes
Crushable=no                ;不能被坦克碾压
; 动画声音
VoiceSelect=BorisSelect
VoiceMove=BorisMove
VoiceAttack=BorisAttackCommand
VoiceFeedback=BorisFear
VoiceSpecialAttack=BorisMove
DieSound=BorisDie
; 移动相关
Locomotor={4A582744-9839-11d1-B709-00A024DDAFD1}
PhysicalSize=1
MovementZone=Infantry
Size=1
; 副武器激光指示器（呼叫米格）
SecondaryFLare=yes
FlareAnim=FLAREMARK
FlareRange=12        ;激光指示器最大标记距离
FlareROF=60          ;冷却帧数
AirstrikeTeam=2      ;普通等级：2架米格
EliteAirstrikeTeam=4 ;精英等级：4架米格
AirstrikeTeamType=MIG;调用飞机类型MIG米格战机

; 普通鲍里斯主武器 AKM
[AKM]
Damage=65
ROF=20
Range=7
Projectile=InvisibleLow
Speed=100
Warhead=BORISWH
Report=BorisAttack
AssaultAnim=UCBLOOD

; 精英鲍里斯主武器 AKME
[AKME]
Damage=90
ROF=20
Range=9
Projectile=InvisibleLow
Speed=100
Warhead=BORISWH
Report=BorisAttack
AssaultAnim=UCBLOOD

; 鲍里斯弹头
[BORISWH]
Verses=1,0.7,0.5,0.3,0.2,0.1,0.5,0.4,0.3,0.1,1
InfDeath=3
Anim=GUNFIRE
Bright=no

)";

int main()
{
    auto ctx = Context(ini);
    auto result = InIParser::Parse(ctx);
    std::print("Result:\n {}\n", result);
}


