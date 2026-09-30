main.floors.MT0 = {
  floorId: "MT0",
  title: "主塔 0 层",
  name: "0",
  canFlyTo: true,
  canUseQuickShop: true,
  cannotViewMap: false,
  defaultGround: "X20016",
  images: [],
  ratio: 1,
  map: [
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 319, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 20016, 10257, 0, 10296, 20016, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 319, 0, 0, 0, 0, 0, 0, 0],
  ],
  firstArrive: null,
  parallelDo: "",
  events: {
    "8,13": [
      "\t[hero]师父教导我：三十六计走为上。",
      { type: "setValue", name: "flag:enemyId", value: "0", norefresh: true },
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleFinish(1);\n}",
      },
      { type: "clearMap" },
      { type: "showHero", time: 0 },
    ],
    "6,13": [
      {
        type: "function",
        function:
          'function(){\ncore.clearMap("uievent", 350, 300, 96, 104);\ncore.drawImage("uievent", "1.png", 96, 0, 150, 104, 100, 140, 150, 104);\nif (core.rand2(6) == 0) {\n\t// 暴击\n\tcore.setFlag("criticalAttack", 1);\n\tcore.fillBoldText("uievent", "会  心  一  击", 168, 232, [255, 255, 255], [0, 0, 0], "20px 黑体");\n} else {\n\tcore.setFlag("criticalAttack", 0);\n}\n}',
      },
      { type: "sleep", time: 100 },
      {
        type: "function",
        function:
          'function(){\n(function () {\n\t// Initialize enemy data\n\tvar heroLevel = core.getFlag("lv");\n\tvar enemyId = core.getFlag("enemyId");\n\tvar realId = enemyId > 10 ? 8 : enemyId;\n\tvar enemyLevel = core.getEnemyLevel(realId);\n\tvar enemyHp = core.getFlag("enemyHp");\n\tvar enemyHpMax = enemyLevel * enemyLevel * 20;\n\tvar enemyPicId = core.getEnemyPictureId(realId);\n\tvar boundingBox = core.getEnemyBoundingBox(enemyPicId);\n\tvar damage = (heroLevel + 6) * heroLevel * heroLevel * core.getBuff("atk") / enemyLevel + core.rand(3);\n\tif (core.getFlag("criticalAttack"))\n\t\tdamage *= 1.7;\n\tdamage = Math.floor(damage);\n\tcore.setFlag("damage", damage);\n\tenemyHp -= damage;\n\tif (enemyHp < 0) enemyHp = 0;\n\tcore.setFlag("enemyHp", enemyHp);\n\t// Monster attacked\n\t// new blood\n\tcore.clearMap("uievent", 142, 22, 300, 12);\n\tcore.fillRect("uievent", 142, 22, 300 * enemyHp / enemyHpMax, 12, [255, 0, 0]);\n\t// new image\n\tcore.clearMap("uievent", 40, 120, 440, 360);\n\tcore.drawImage("uievent", "1.png", 0, 0, 96, 104, 350, 300, 96, 104);\n\tcore.drawImage("uievent", enemyPicId.toString() + ".png", boundingBox[0], boundingBox[1], boundingBox[2], boundingBox[3], 40, 120, boundingBox[2], boundingBox[3]);\n\t// new text\n\tcore.fillText("uievent", "-" + damage.toString(), 80, 112, [255, 255, 0], "16px 黑体");\n\tif (core.getFlag("criticalAttack"))\n\t\tcore.fillBoldText("uievent", "会  心  一  击", 168, 232, [255, 255, 255], [0, 0, 0], "20px 黑体");\n})();\n}',
      },
      { type: "sleep", time: 100 },
      {
        type: "function",
        function:
          'function(){\ncore.clearMap("uievent", 80, 90, 80, 30);\ncore.fillText("uievent", "-" + core.getFlag("damage").toString(), 80, 108, [255, 255, 0], "16px 黑体");\n}',
      },
      { type: "sleep", time: 100 },
      {
        type: "function",
        function:
          'function(){\ncore.clearMap("uievent", 80, 90, 80, 30);\ncore.fillText("uievent", "-" + core.getFlag("damage").toString(), 80, 104, [255, 255, 0], "16px 黑体");\n}',
      },
      { type: "sleep", time: 100 },
      {
        type: "if",
        condition: "(flag:enemyHp==0)",
        true: [
          {
            type: "function",
            function:
              'function(){\n(function () {\n\tvar enemyId = core.getFlag("enemyId");\n\tvar realId = enemyId > 10 ? 8 : enemyId;\n\tif (realId < enemyId) return;\n\tvar money = core.getEnemyMoney(realId);\n\tvar items = core.getEnemyItem(realId);\n\tvar experience = core.getEnemyExperience(realId);\n\tvar winMsg = "战斗胜利。得到" + money.toString() + "两银子";\n\tcore.status.hero.money += money;\n\tcore.status.hero.exp += experience;\n\tfor (var i in items) {\n\t\tcore.setItem(i, core.itemCount(i) + items[i]);\n\t\twinMsg += "，" + items[i].toString() + "个" + core.material.items[i].name;\n\t}\n\twinMsg += ",获得" + experience.toString() + "点经验。";\n\tswitch (enemyId) {\n\tcase 0:\n\t\tcore.insertAction("\\t[大师兄,role2.png]小师弟你的武功长进真快，连我都不是你的对手了。");\n\t\tcore.setFlag("brother", 1);\n\t\tcore.setBlock("X20016", 40, 3, "MT3");\n\t\tcore.hideBlock(40, 4, "MT3");\n\t\tcore.setBlock("X20067", 14, 12, "MT1");\n\t\tcore.setBlock("X20081", 14, 13, "MT1");\n\t\tcore.showBlock(14, 13, "MT1");\n\t\tbreak;\n\tcase 7:\n\t\tcore.setFlag("child", 2);\n\t\tbreak;\n\tcase 8:\n\t\tcore.setFlag("ret1", 1);\n\t\tbreak;\n\tcase 9:\n\t\tcore.setFlag("ret2", 1);\n\t\tbreak;\n\tcase 10:\n\t\tcore.removeBlock(36, 54, "MT6");\n\t\tbreak;\n\t}\n\tcore.insertAction(winMsg);\n})();\n}',
          },
          { type: "showHero", time: 0 },
          {
            type: "function",
            async: true,
            function: "function(){\ncore.battleFinish();\n}",
          },
        ],
        false: [
          {
            type: "function",
            function:
              'function(){\n(function () {\n\t// Initialize enemy data\n\tvar enemyId = core.getFlag("enemyId");\n\tvar realId = enemyId > 10 ? 8 : enemyId;\n\tvar enemyPicId = core.getEnemyPictureId(realId);\n\tvar boundingBox = core.getEnemyAttackBoundingBox(enemyPicId);\n\t// new image\n\tcore.clearMap("uievent", 80, 90, 80, 30);\n\tcore.clearMap("uievent", 40, 120, 310, 180);\n\tcore.drawImage("uievent", enemyPicId.toString() + ".png", boundingBox[0], boundingBox[1], boundingBox[2], boundingBox[3], 100, 160, boundingBox[2], boundingBox[3]);\n})();\n}',
          },
          { type: "sleep", time: 100 },
          {
            type: "function",
            function:
              'function(){\n(function () {\n\t// Initialize enemy data\n\tvar heroLevel = core.getFlag("lv");\n\tvar enemyId = core.getFlag("enemyId");\n\tvar realId = enemyId > 10 ? 8 : enemyId;\n\tvar enemyLevel = core.getEnemyLevel(realId);\n\tvar enemyPicId = core.getEnemyPictureId(realId);\n\tvar boundingBox = core.getEnemyBoundingBox(enemyPicId);\n\tvar damage = enemyLevel * enemyLevel * 17 / heroLevel * (2 - core.getBuff("def")) + core.rand(4);\n\tdamage = Math.floor(damage);\n\tcore.setFlag("damage", damage);\n\tcore.status.hero.hp -= damage;\n\tif (core.status.hero.hp < 0) core.status.hero.hp = 0;\n\t// Hero attacked\n\t// new blood\n\tcore.clearMap("uievent", 142, 64, 300, 12);\n\tcore.fillRect("uievent", 142, 64, 300 * core.status.hero.hp / core.status.hero.hpmax, 12, [255, 0, 0]);\n\t// new image\n\tcore.clearMap("uievent", 40, 120, 440, 360);\n\tcore.drawImage("uievent", "1.png", 0, 0, 96, 104, 350, 300, 96, 104);\n\tcore.drawImage("uievent", enemyPicId.toString() + ".png", boundingBox[0], boundingBox[1], boundingBox[2], boundingBox[3], 40, 120, boundingBox[2], boundingBox[3]);\n\t// new text\n\tcore.fillText("uievent", "-" + damage.toString(), 370, 292, [255, 255, 0], "16px 黑体");\n})();\n}',
          },
          { type: "sleep", time: 100 },
          {
            type: "function",
            function:
              'function(){\ncore.clearMap("uievent", 370, 270, 80, 30);\ncore.fillText("uievent", "-" + core.getFlag("damage").toString(), 370, 288, [255, 255, 0], "16px 黑体");\n}',
          },
          { type: "sleep", time: 100 },
          {
            type: "function",
            function:
              'function(){\ncore.clearMap("uievent", 370, 270, 80, 30);\ncore.fillText("uievent", "-" + core.getFlag("damage").toString(), 370, 284, [255, 255, 0], "16px 黑体");\n}',
          },
          { type: "sleep", time: 100 },
          {
            type: "if",
            condition: "(core.status.hero.hp==0)",
            true: [
              {
                type: "function",
                function:
                  'function(){\n(function () {\n\tif (core.getFlag("enemyId") == 0) core.insertAction("小师弟你的功力还差的远，还是先踏踏实实地练功吧。");\n\telse core.insertAction("看来我还需要好好练功。");\n})();\n}',
              },
              {
                type: "setValue",
                name: "flag:enemyId",
                value: "0",
                norefresh: true,
              },
              { type: "showHero", time: 0 },
              {
                type: "function",
                async: true,
                function: "function(){\ncore.battleFinish(1);\n}",
              },
            ],
            false: [
              {
                type: "function",
                function:
                  'function(){\ncore.clearMap("uievent", 370, 270, 80, 30);\n}',
              },
            ],
          },
        ],
      },
    ],
  },
  changeFloor: {},
  afterBattle: {},
  afterGetItem: {},
  afterOpenDoor: {},
  cannotMove: {},
  bgmap: [],
  fgmap: [],
  width: 15,
  height: 15,
  autoEvent: {},
  eachArrive: [
    { type: "hideHero", time: 0 },
    { type: "autoSave", nohint: true },
    {
      type: "function",
      function:
        'function(){\n(function () {\n\t// Initialize enemy data\n\tvar enemyId = core.getFlag("enemyId");\n\tvar realId = enemyId > 10 ? 8 : enemyId;\n\tvar enemyLevel = core.getEnemyLevel(realId);\n\tvar enemyHp = enemyLevel * enemyLevel * 20;\n\tcore.setFlag("enemyHp", enemyHp);\n\tvar enemyName = core.getEnemyName(realId);\n\tvar enemyPicId = core.getEnemyPictureId(realId);\n\tvar boundingBox = core.getEnemyBoundingBox(enemyPicId);\n\t// Draw ui\n\tcore.ui._createUIEvent();\n\t// Blood of monster\n\tcore.fillBoldText("uievent", enemyName, 32, 38, [0, 0, 0, 1], [255, 255, 255], "28px 黑体");\n\tcore.strokeRect("uievent", 141, 21, 302, 14, [0, 0, 0], 2);\n\tcore.fillRect("uievent", 142, 22, 300, 12, [255, 0, 0]);\n\t// Blood of hero\n\tcore.fillBoldText("uievent", core.status.hero.name, 32, 80, [0, 0, 0, 1], [255, 255, 255], "28px 黑体");\n\tcore.strokeRect("uievent", 141, 63, 302, 14, [0, 0, 0], 2);\n\tcore.fillRect("uievent", 142, 64, 300 * core.status.hero.hp / core.status.hero.hpmax, 12, [255, 0, 0]);\n\t// Monster image\n\tcore.drawImage("uievent", enemyPicId.toString() + ".png", boundingBox[0], boundingBox[1], boundingBox[2], boundingBox[3], 40, 120, boundingBox[2], boundingBox[3]);\n\t// Hero image\n\tcore.drawImage("uievent", "1.png", 0, 0, 96, 104, 350, 300, 96, 104);\n})();\n}',
    },
  ],
};

main.floors.MT1 = {
  floorId: "MT1",
  title: "主塔 1 层",
  name: "1",
  width: 19,
  height: 20,
  canFlyTo: true,
  canUseQuickShop: true,
  cannotViewMap: false,
  cannotMoveDirectly: false,
  images: [],
  ratio: 1,
  defaultGround: "X20043",
  firstArrive: [
    "\t[hero]十年练剑，终日习武。师父说过，好男儿志在四方。可我每日只在这山上，纵有一身好武艺，又有何用！听说外面的世界很精彩，我去恳请师父让我下山！",
  ],
  eachArrive: null,
  parallelDo: "",
  events: {
    "9,9": [
      {
        type: "switch",
        condition: "flag:master",
        caseList: [
          {
            case: "0",
            action: [
              "\t[师父,role1.png]孩子，你要好好练功！不要急于下山！只有战胜你的大师兄，我才会让你下山的。",
              {
                type: "setValue",
                name: "flag:master",
                value: "1",
                norefresh: true,
              },
            ],
          },
          {
            case: "1",
            action: [
              {
                type: "if",
                condition: "flag:brother",
                true: [
                  "\t[师父,role1.png]孩子，其实你是个孤儿。我捡到你的时候，你的身边还有一把金赤剑和一封遗书。遗书是用一种特殊的文字写成。你可以到京城里找司马一先生，他博学多才，也许能读懂。路上小心！",
                  "\t[师父,role1.png]沿着东边的路可以到京城。",
                  {
                    type: "setValue",
                    name: "flag:master",
                    value: "2",
                    norefresh: true,
                  },
                ],
                false: ["\t[师父,role1.png]要好好练功啊，不要偷懒！"],
              },
            ],
          },
          {
            case: "2",
            action: [
              "\t[师父,role1.png]你有没有找到司马一？知道你自己的身世了吗？",
            ],
          },
        ],
      },
    ],
    "0,13": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [
        {
          type: "if",
          condition: "flag:master",
          true: [{ type: "changeFloor", floorId: "MT2", loc: [18, 14] }],
          false: ["\t[hero]师父正找我呢，是不是又有好吃的糕点给我？"],
        },
      ],
    },
    "18,14": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [
        {
          type: "if",
          condition: "(flag:master<2)",
          true: [
            "\t[hero]很想下山去闯荡，可是没练好武功前，师父是不会答应的。",
          ],
          false: [
            {
              type: "changeFloor",
              floorId: "MT4",
              loc: [1, 10],
              direction: "right",
            },
          ],
        },
      ],
    },
    "2,14": [
      "西边的树林中有一处神秘泉水，据说喝了可以使人起死回生。去北边的树林要小心，那里有很多怪物！",
    ],
    "15,13": ["通往山下的路。"],
    "6,7": [
      {
        type: "if",
        condition: "flag:pot1",
        true: ["已经检查过了啊！！！"],
        false: [
          {
            type: "setValue",
            name: "item:redPotion",
            value: "(item:redPotion+1)",
            norefresh: true,
          },
          "得到一个止血草。",
          "\t[hero]咦？！谁把宝贝藏在水缸里？？嘻嘻，运气不错！",
          { type: "setValue", name: "flag:pot1", value: "1", norefresh: true },
        ],
      },
    ],
    "14,13": [
      {
        type: "if",
        condition: "flag:brother",
        true: [
          "\t[小师妹,role3.png]师兄，你真的要下山了？！你可要好好照顾自己，京城里坏人很多，尤其要小心那些走江湖的女生！",
        ],
      },
    ],
  },
  changeFloor: {},
  afterBattle: {},
  afterGetItem: {},
  afterOpenDoor: {},
  autoEvent: {},
  cannotMove: {},
  map: [
    [
      20016, 20016, 20016, 20015, 20016, 20016, 20042, 20042, 20042, 20015,
      20042, 20042, 20042, 20015, 20042, 20016, 20045, 20002, 20002,
    ],
    [
      20042, 20015, 20015, 20015, 20015, 20042, 20042, 20015, 20015, 331, 20015,
      20015, 20015, 20015, 20015, 20016, 20016, 20045, 20045,
    ],
    [
      20042, 20042, 20015, 20042, 20042, 20042, 20015, 20029, 20000, 20002,
      20014, 20015, 20015, 20042, 20016, 20016, 20015, 20015, 20016,
    ],
    [
      20015, 20042, 20042, 20042, 20042, 20042, 20015, 20000, 20002, 20002,
      20029, 20015, 20042, 20042, 20016, 20015, 20016, 20015, 20015,
    ],
    [
      20015, 20015, 20015, 20015, 20015, 20016, 20015, 20000, 20001, 20002,
      20029, 20015, 20042, 20042, 20016, 20016, 20015, 20015, 20015,
    ],
    [
      20000, 20014, 20029, 20000, 20029, 20000, 20029, 20015, 20014, 20002,
      20002, 20015, 20042, 20042, 20042, 20015, 20015, 20015, 20016,
    ],
    [
      20014, 20029, 20043, 20006, 20007, 20008, 20043, 20003, 20004, 20005,
      20043, 20014, 20016, 20042, 20042, 20042, 20016, 20016, 20016,
    ],
    [
      20015, 20014, 20043, 20020, 20021, 20022, 20071, 20017, 20018, 20019,
      20043, 20015, 20000, 20015, 20042, 20042, 20042, 20042, 20042,
    ],
    [
      20016, 20015, 20072, 20043, 20043, 20043, 20043, 20043, 20043, 20065,
      20043, 20029, 20029, 20000, 20015, 20042, 20042, 20015, 20015,
    ],
    [
      20042, 20015, 20029, 20043, 20043, 20043, 20043, 20043, 20043, 20079,
      20043, 20014, 20029, 20029, 20000, 20015, 20016, 20042, 20015,
    ],
    [
      20042, 20015, 20029, 20029, 20029, 20029, 20029, 20043, 20043, 20043,
      20043, 20001, 20014, 20014, 20000, 20000, 20015, 20042, 20042,
    ],
    [
      20042, 20015, 20015, 20000, 20000, 20029, 20029, 20043, 20043, 20043,
      20043, 20002, 20001, 20002, 20014, 20000, 20015, 20042, 20042,
    ],
    [
      20042, 20042, 20015, 20015, 20043, 20043, 20043, 20002, 20002, 20002,
      20002, 20002, 20002, 20002, 20014, 20029, 20015, 20016, 20015,
    ],
    [
      20002, 20043, 20043, 20043, 20043, 20014, 20014, 20000, 20014, 20002,
      20001, 20002, 20002, 20002, 20002, 20031, 20014, 20014, 20014,
    ],
    [
      20000, 20015, 20031, 20000, 20000, 20000, 20015, 20029, 20014, 20002,
      20002, 20002, 20002, 20001, 20002, 20002, 20002, 20002, 20002,
    ],
    [
      20015, 20015, 20015, 20045, 20015, 20015, 20015, 20015, 20000, 20014,
      20002, 20002, 20002, 20002, 20002, 20002, 20002, 20002, 20015,
    ],
    [
      20015, 20016, 20016, 20015, 20045, 20045, 20045, 20015, 20015, 20000,
      20014, 20014, 20001, 20002, 20002, 20014, 20000, 20014, 20015,
    ],
    [
      20016, 20016, 20016, 20042, 20042, 20042, 20042, 20045, 20045, 20015,
      20016, 20000, 20014, 20014, 20014, 20000, 20014, 20015, 20045,
    ],
    [
      20015, 20016, 20016, 20015, 20016, 20042, 20042, 20042, 20045, 20015,
      20015, 20016, 20000, 20015, 20029, 20000, 20015, 20015, 20045,
    ],
    [
      20015, 20015, 20015, 20015, 20015, 20042, 20042, 20042, 20042, 20045,
      20045, 20015, 20015, 20045, 20045, 20045, 20015, 20045, 20045,
    ],
  ],
  bgmap: [],
  fgmap: [],
};
main.floors.MT2 = {
  floorId: "MT2",
  title: "主塔 2 层",
  name: "2",
  width: 20,
  height: 29,
  canFlyTo: true,
  canUseQuickShop: true,
  cannotViewMap: false,
  cannotMoveDirectly: false,
  images: [],
  ratio: 1,
  defaultGround: "X20001",
  firstArrive: [],
  eachArrive: [],
  parallelDo: "",
  events: {
    "5,7": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(1);\n}",
      },
    ],
    "4,13": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(1);\n}",
      },
    ],
    "5,21": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(1);\n}",
      },
    ],
    "4,26": [
      "生命恢复",
      { type: "setValue", name: "status:hp", value: "status:hpmax" },
    ],
    "5,27": [
      "生命恢复",
      { type: "setValue", name: "status:hp", value: "status:hpmax" },
    ],
    "11,2": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(1);\n}",
      },
    ],
    "16,3": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(1);\n}",
      },
    ],
    "13,8": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(1);\n}",
      },
    ],
    "16,14": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(1);\n}",
      },
    ],
    "19,14": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [{ type: "changeFloor", floorId: "MT1", loc: [1, 13] }],
    },
    "18,0": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [{ type: "changeFloor", floorId: "MT3", loc: [9, 26] }],
    },
  },
  changeFloor: {},
  afterBattle: {},
  afterGetItem: {},
  afterOpenDoor: {},
  autoEvent: {},
  cannotMove: { "19,14": [] },
  map: [
    [
      20016, 20016, 20016, 20015, 20015, 20015, 20015, 20042, 20042, 20016,
      20030, 20029, 20014, 20000, 20015, 20015, 20016, 20045, 20002, 20045,
    ],
    [
      20016, 20016, 20016, 20015, 20015, 20042, 20042, 20042, 20015, 20042,
      20029, 20014, 20002, 331, 20042, 20042, 20015, 20045, 20002, 20045,
    ],
    [
      20016, 20016, 20015, 20016, 20015, 20042, 20042, 20042, 20042, 20015,
      20029, 20002, 20002, 20015, 20042, 20042, 20042, 20045, 20002, 20045,
    ],
    [
      20016, 20016, 20016, 20015, 20016, 20042, 20042, 20015, 20015, 20015,
      20015, 20002, 20016, 20042, 20042, 20016, 20002, 20002, 20002, 20045,
    ],
    [
      20015, 20016, 20015, 20015, 20015, 20000, 20000, 20042, 20015, 20016,
      20029, 20002, 20016, 20015, 20042, 20016, 20002, 20045, 20045, 20015,
    ],
    [
      20016, 20015, 20016, 20016, 20014, 20014, 20000, 20014, 20014, 20000,
      20000, 20002, 20001, 20016, 20015, 20002, 20002, 20045, 20015, 20015,
    ],
    [
      20015, 20016, 20016, 20014, 20000, 20002, 20002, 20002, 20002, 20002,
      20002, 20002, 20002, 20002, 20002, 20001, 20002, 20045, 20015, 20015,
    ],
    [
      20015, 20015, 20014, 20014, 20002, 20002, 20002, 20002, 20002, 20045,
      20045, 20045, 20002, 20002, 20002, 20002, 20045, 20042, 20015, 20015,
    ],
    [
      20042, 20014, 20029, 20002, 20002, 20001, 20000, 20000, 20016, 20015,
      20016, 20015, 20002, 20002, 20002, 20045, 20045, 20042, 20016, 20016,
    ],
    [
      20015, 20014, 20029, 20002, 20002, 20000, 20000, 20015, 20016, 20016,
      20015, 20016, 20001, 20002, 20045, 20015, 20015, 20015, 20016, 20015,
    ],
    [
      20016, 20015, 20029, 20002, 20002, 20015, 20015, 20015, 20015, 20015,
      20015, 20015, 20002, 20002, 20045, 20015, 20042, 20042, 20042, 20042,
    ],
    [
      20015, 20016, 20029, 20002, 20002, 20000, 20015, 20042, 20042, 20042,
      20016, 20015, 20002, 20002, 20002, 20016, 20015, 20042, 20042, 20042,
    ],
    [
      20015, 20015, 20000, 20002, 20001, 20002, 20000, 20016, 20015, 20042,
      20015, 20000, 20002, 20002, 20001, 20002, 20001, 20016, 20015, 20042,
    ],
    [
      20016, 20016, 20000, 20002, 20002, 20001, 20015, 20015, 20015, 20015,
      20015, 20000, 20000, 20015, 20000, 20002, 20002, 20002, 20002, 20015,
    ],
    [
      20016, 20015, 20000, 20000, 20002, 20002, 20029, 20015, 20042, 20015,
      20016, 20015, 20015, 20015, 20015, 20000, 20000, 20002, 20002, 20002,
    ],
    [
      20015, 20015, 20016, 20000, 20001, 20002, 20029, 20000, 20015, 20016,
      20015, 20016, 20015, 20042, 20042, 20015, 20015, 20000, 20015, 20000,
    ],
    [
      20015, 20015, 20000, 20002, 20002, 20001, 20002, 20000, 20015, 20015,
      20016, 20016, 20042, 20042, 20042, 20042, 20015, 20015, 20015, 20015,
    ],
    [
      20015, 20015, 20029, 20002, 20002, 20002, 20001, 20015, 20016, 20016,
      20042, 20042, 20042, 20042, 20042, 20016, 20016, 20015, 20016, 20016,
    ],
    [
      20016, 20015, 20029, 20002, 20001, 20002, 20014, 20016, 20015, 20015,
      20042, 20042, 20042, 20042, 20042, 20016, 20016, 20015, 20016, 20016,
    ],
    [
      20015, 20029, 20002, 20001, 20002, 20014, 20016, 20015, 20015, 20016,
      20042, 20042, 20042, 20042, 20042, 20015, 20015, 20015, 20015, 20016,
    ],
    [
      20015, 20029, 20029, 20002, 20002, 20002, 20014, 20016, 20016, 20015,
      20015, 20016, 20042, 20042, 20042, 20016, 20015, 20016, 20015, 20015,
    ],
    [
      20015, 20016, 20029, 20002, 20001, 20002, 20029, 20014, 20016, 20015,
      20016, 20015, 20016, 20042, 20042, 20016, 20016, 20016, 20016, 20015,
    ],
    [
      20016, 20015, 20000, 20002, 20001, 20002, 20029, 20029, 20029, 20014,
      20015, 20015, 20016, 20042, 20042, 20042, 20016, 20016, 20016, 20016,
    ],
    [
      20016, 20015, 20001, 20002, 20002, 20002, 20001, 20002, 20029, 20014,
      20015, 20015, 20016, 20042, 20042, 20042, 20042, 20016, 20015, 20015,
    ],
    [
      20016, 20015, 20029, 20002, 20002, 20001, 20001, 20002, 20000, 20014,
      20016, 20016, 20016, 20042, 20042, 20042, 20042, 20042, 20016, 20015,
    ],
    [
      20016, 20015, 20029, 20029, 20002, 20002, 20002, 20002, 20000, 20015,
      20015, 20015, 20016, 20016, 20016, 20042, 20042, 20042, 20042, 20042,
    ],
    [
      20016, 20015, 20015, 20002, 20028, 20001, 20002, 20029, 20015, 20015,
      20016, 20015, 20016, 20016, 20016, 20015, 20042, 20042, 20042, 20042,
    ],
    [
      20016, 20016, 20015, 20015, 20028, 20028, 20015, 20029, 20016, 20016,
      20015, 20016, 20016, 20015, 20016, 20016, 20015, 20042, 20042, 20015,
    ],
    [
      20015, 20016, 20016, 20015, 20028, 20028, 20015, 20029, 20016, 20015,
      20015, 20015, 20016, 20016, 20015, 20015, 20014, 20014, 20015, 20000,
    ],
  ],
  bgmap: [],
  fgmap: [],
};
main.floors.MT3 = {
  floorId: "MT3",
  title: "主塔 3 层",
  name: "3",
  width: 46,
  height: 28,
  canFlyTo: true,
  canUseQuickShop: true,
  cannotViewMap: false,
  cannotMoveDirectly: false,
  images: [],
  ratio: 1,
  defaultGround: "X20002",
  firstArrive: [],
  eachArrive: [],
  parallelDo: "",
  events: {
    "6,21": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(1);\n}",
      },
    ],
    "3,19": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(2);\n}",
      },
    ],
    "3,12": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(1);\n}",
      },
    ],
    "9,12": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(2);\n}",
      },
    ],
    "10,8": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(2);\n}",
      },
    ],
    "9,5": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(1);\n}",
      },
    ],
    "5,4": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(2);\n}",
      },
    ],
    "19,16": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(2);\n}",
      },
    ],
    "24,17": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(2);\n}",
      },
    ],
    "19,20": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(2);\n}",
      },
    ],
    "13,22": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(1);\n}",
      },
    ],
    "28,24": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(3);\n}",
      },
    ],
    "38,21": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(3);\n}",
      },
    ],
    "38,15": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(3);\n}",
      },
    ],
    "39,9": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(3);\n}",
      },
    ],
    "26,4": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(2);\n}",
      },
    ],
    "28,12": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(2);\n}",
      },
    ],
    "40,4": [
      {
        type: "if",
        condition: "(flag:brother==0)",
        true: [
          "\t[hero]大师兄我来向你挑战！",
          {
            type: "function",
            async: true,
            function: "function(){\ncore.battleStart(0, 1);\n}",
          },
        ],
      },
    ],
    "9,27": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [{ type: "changeFloor", floorId: "MT2", loc: [18, 1] }],
    },
  },
  changeFloor: {},
  afterBattle: {},
  afterGetItem: {},
  afterOpenDoor: {},
  autoEvent: {},
  cannotMove: {},
  map: [
    [
      20016, 20016, 20016, 20016, 20015, 20042, 20015, 20042, 20042, 20042,
      20042, 20016, 20016, 20016, 20016, 20016, 20016, 20016, 20016, 20016,
      20016, 20016, 20016, 20016, 20016, 20016, 20016, 20015, 20015, 20015,
      20015, 20015, 20016, 20016, 20015, 20016, 20016, 20016, 20016, 20016,
      20016, 20015, 20016, 20015, 20016, 20028,
    ],
    [
      20016, 20016, 20016, 20016, 20016, 20015, 20042, 20042, 20042, 20015,
      20042, 20042, 20042, 20016, 20016, 20016, 20016, 20015, 20016, 20016,
      20016, 20016, 20016, 20015, 20016, 20015, 20015, 20015, 20015, 20015,
      20014, 20016, 20015, 20015, 20015, 20015, 20016, 20015, 20016, 20016,
      20015, 20016, 20014, 20001, 337, 20028,
    ],
    [
      20042, 20042, 20042, 20042, 20015, 20015, 20015, 20015, 20015, 20042,
      20015, 20042, 20042, 20015, 20016, 20016, 20015, 20042, 20015, 20016,
      20042, 20042, 20015, 20016, 20015, 20014, 20015, 20029, 20029, 20014,
      20016, 20016, 20029, 20015, 20014, 20015, 20015, 20015, 20015, 20015,
      20015, 20015, 20001, 20001, 20029, 20015,
    ],
    [
      20042, 20015, 20015, 20015, 20015, 20002, 20002, 20015, 20042, 20015,
      20042, 20042, 20015, 20042, 20015, 20042, 20042, 20015, 20042, 20016,
      20042, 20042, 20016, 20015, 20029, 20029, 20001, 20002, 20002, 20002,
      20001, 20002, 20002, 20029, 20014, 20002, 20015, 20014, 20029, 20015,
      20039, 20001, 20001, 20016, 20045, 20045,
    ],
    [
      20042, 20045, 334, 20002, 20002, 20002, 20002, 20002, 20015, 20015, 20015,
      20042, 20042, 20015, 20042, 20042, 20015, 20015, 20015, 20042, 20042,
      20042, 20015, 20015, 20015, 20029, 20002, 20002, 20002, 20001, 20002,
      20002, 20002, 20002, 20002, 20002, 20002, 20002, 20002, 20029, 20053,
      20001, 20045, 20045, 20015, 20015,
    ],
    [
      20042, 20015, 20045, 20029, 20029, 20002, 20002, 20001, 20002, 20002,
      20002, 20000, 20015, 20015, 20015, 20042, 20016, 20042, 20042, 20042,
      20016, 20016, 20016, 20016, 20029, 20002, 20001, 20045, 20045, 20014,
      20014, 20001, 20002, 20002, 20002, 20001, 20002, 20002, 20002, 20002,
      20002, 20002, 20014, 20015, 20016, 20016,
    ],
    [
      20015, 20042, 20015, 20045, 20045, 20029, 20002, 20002, 20002, 20002,
      20002, 20002, 20029, 20000, 20016, 20016, 20015, 20042, 20042, 20016,
      20016, 20014, 20029, 20014, 20002, 20002, 20002, 20045, 20042, 20015,
      20045, 20029, 20014, 20029, 20014, 20002, 20002, 20001, 20002, 20002,
      20002, 20002, 20029, 20014, 20029, 20015,
    ],
    [
      20042, 20015, 20016, 20045, 20000, 20029, 20029, 20002, 20002, 20001,
      20002, 20002, 20002, 20029, 20015, 20015, 20015, 20015, 20016, 20016,
      20029, 20001, 20002, 20001, 20002, 20002, 20045, 20045, 20015, 20042,
      20042, 20045, 20045, 20045, 20045, 20014, 20029, 20002, 20001, 20002,
      20002, 20002, 20002, 20014, 20016, 20015,
    ],
    [
      20015, 20015, 20015, 20016, 20045, 20000, 20029, 20029, 20002, 20002,
      20002, 20001, 20002, 20000, 20015, 20015, 20016, 20016, 20015, 20002,
      20002, 20002, 20002, 20002, 20045, 20045, 20042, 20015, 20015, 20015,
      20042, 20042, 20042, 20015, 20045, 20045, 20014, 20002, 20002, 20002,
      20001, 20001, 20014, 20045, 20045, 20016,
    ],
    [
      20042, 20030, 20016, 20016, 20030, 20045, 20045, 20045, 20000, 20002,
      20002, 20002, 20002, 20002, 20015, 20016, 20016, 20045, 20002, 20002,
      20002, 20045, 20002, 20002, 20045, 20029, 20029, 20042, 20042, 20042,
      20042, 20042, 20042, 20042, 20015, 20016, 20045, 20002, 20002, 20001,
      20002, 20001, 20029, 20029, 20015, 20015,
    ],
    [
      20042, 20016, 20030, 20016, 20015, 20015, 20015, 20015, 20000, 20002,
      20002, 20002, 20002, 20045, 20015, 20015, 20016, 20045, 20002, 20002,
      20001, 20045, 20045, 20002, 20002, 20002, 20029, 20029, 20029, 20029,
      20015, 20016, 20042, 20015, 20015, 20015, 20045, 20002, 20002, 20002,
      20002, 20002, 20029, 20015, 20015, 20016,
    ],
    [
      20042, 20016, 20001, 20002, 20002, 20015, 20000, 20000, 20000, 20002,
      20001, 20001, 20002, 20015, 20015, 20015, 20015, 20045, 20045, 332, 20002,
      20002, 20045, 20001, 20002, 20002, 20002, 20002, 20002, 20002, 20014,
      20045, 20016, 20042, 20015, 20015, 20045, 20002, 20002, 20002, 20002,
      20001, 20029, 20015, 20016, 20042,
    ],
    [
      20015, 20002, 20002, 20001, 20002, 20002, 20002, 20002, 20002, 20002,
      20002, 20001, 20002, 20029, 20015, 20016, 20016, 20015, 20045, 20045,
      20045, 20045, 20016, 20045, 20045, 20002, 20002, 20001, 20002, 20002,
      20002, 20045, 20042, 20015, 20014, 20014, 20002, 20002, 20001, 20002,
      20002, 20029, 20045, 20016, 20042, 20042,
    ],
    [
      20000, 20045, 20002, 20002, 20002, 20002, 20002, 20002, 20002, 20002,
      20001, 20002, 20002, 20045, 20016, 20016, 20015, 20015, 20015, 20015,
      20015, 20015, 20015, 20015, 20015, 20045, 20045, 20002, 20001, 20002,
      20014, 20045, 20015, 20042, 20015, 20014, 20002, 20001, 20001, 20002,
      20002, 20029, 20045, 20016, 20042, 20042,
    ],
    [
      20015, 20030, 20045, 20045, 20002, 20001, 20002, 20002, 20045, 20045,
      20002, 20002, 20045, 20042, 20015, 20015, 20015, 20015, 20015, 20029,
      20029, 20029, 20015, 20015, 20029, 20029, 20029, 20002, 20002, 20002,
      20045, 20042, 20042, 20015, 20014, 20002, 20002, 20002, 20002, 20001,
      20045, 20045, 20015, 20016, 20042, 20042,
    ],
    [
      20042, 20030, 20015, 20045, 20002, 20001, 20045, 20045, 20016, 20045,
      20002, 20001, 20045, 20015, 20015, 20015, 20014, 20002, 20002, 20002,
      20029, 20015, 20015, 20015, 20015, 20002, 20002, 20001, 20002, 20002,
      20045, 20042, 20015, 20015, 20015, 20014, 20002, 20002, 20001, 20002,
      20014, 20015, 20015, 20042, 20042, 20042,
    ],
    [
      20015, 20030, 20029, 20002, 20002, 20002, 20045, 20015, 20016, 20045,
      20002, 20002, 20001, 20029, 20029, 20002, 20002, 20002, 20001, 20002,
      20002, 20002, 20015, 20029, 20002, 20002, 20002, 20002, 20001, 20014,
      20045, 20042, 20042, 20016, 20016, 20045, 20045, 20045, 20002, 20002,
      20002, 20015, 20042, 20042, 20042, 20042,
    ],
    [
      20030, 20014, 20002, 20001, 20029, 20015, 20015, 20015, 20015, 20042,
      20045, 20045, 20002, 20002, 20002, 20002, 20002, 20001, 20001, 20001,
      20002, 20002, 20002, 20001, 20002, 20002, 20045, 20045, 20045, 20045,
      20045, 20042, 20042, 20016, 20016, 20015, 20016, 20045, 20002, 20001,
      20002, 20002, 20015, 20042, 20016, 20016,
    ],
    [
      20015, 20016, 20001, 20002, 20014, 20045, 20015, 20042, 20042, 20042,
      20042, 20015, 20045, 20045, 20045, 20045, 20045, 20045, 20002, 20001,
      20001, 20002, 20002, 20002, 20002, 20045, 20016, 20016, 20015, 20016,
      20016, 20042, 20042, 20016, 20015, 20016, 20015, 20016, 20045, 20002,
      20002, 20014, 20015, 20042, 20042, 20015,
    ],
    [
      20016, 20030, 20002, 20002, 20014, 20045, 20015, 20042, 20015, 20015,
      20015, 20015, 20015, 20016, 20015, 20016, 20016, 20016, 20002, 20002,
      20001, 20001, 20002, 20002, 20045, 20016, 20016, 20015, 20016, 20015,
      20016, 20042, 20042, 20015, 20016, 20015, 20015, 20015, 20045, 20002,
      20001, 20045, 20042, 20042, 20016, 20016,
    ],
    [
      20015, 20016, 20002, 20002, 20002, 20014, 20015, 20015, 20014, 20014,
      20015, 20014, 20015, 20015, 20015, 20015, 20002, 20002, 20002, 20002,
      20001, 20002, 20014, 20045, 20015, 20015, 20015, 20016, 20015, 20016,
      20015, 20042, 20042, 20016, 20015, 20016, 20016, 20016, 20002, 20001,
      20002, 20045, 20042, 20042, 20016, 20015,
    ],
    [
      20042, 20030, 20014, 20002, 20001, 20002, 20002, 20001, 20002, 20014,
      20014, 20014, 20014, 20015, 20016, 20001, 20002, 20002, 20002, 20002,
      20002, 20001, 20014, 20045, 20002, 20001, 20002, 20015, 20015, 20015,
      20014, 20015, 20042, 20015, 20015, 20015, 20015, 20002, 20001, 20002,
      20045, 20045, 20042, 20042, 20042, 20016,
    ],
    [
      20015, 20030, 20000, 20029, 20002, 20000, 20002, 20002, 20002, 20002,
      20002, 20001, 20002, 20002, 20002, 20002, 20002, 20002, 20045, 20045,
      20002, 20002, 20001, 20002, 20002, 20002, 20002, 20002, 20002, 20002,
      20001, 20029, 20014, 20029, 20014, 20002, 20001, 20001, 20002, 20045,
      20016, 20016, 20016, 20042, 20042, 20042,
    ],
    [
      20016, 20030, 20042, 20015, 20042, 20042, 20000, 20002, 20002, 20002,
      20002, 20002, 20002, 20002, 20002, 20002, 20002, 20002, 20045, 20016,
      20045, 20002, 20002, 20002, 20002, 20002, 20001, 20002, 20002, 20001,
      20002, 20002, 20002, 20001, 20002, 20002, 20002, 20014, 20045, 20016,
      20016, 20016, 20030, 20016, 20042, 20042,
    ],
    [
      20015, 20016, 20042, 20015, 20015, 20000, 20000, 20000, 20002, 20001,
      20002, 20045, 20045, 20045, 20002, 20002, 20002, 20045, 20016, 20016,
      20045, 20045, 20045, 20002, 20002, 20001, 20002, 20002, 20002, 20001,
      20002, 20001, 20045, 20045, 20001, 20045, 20045, 20045, 20045, 20016,
      20030, 20016, 20016, 20016, 20042, 20042,
    ],
    [
      20016, 20030, 20029, 20014, 20000, 20015, 20015, 20016, 20045, 20002,
      20045, 20045, 20016, 20016, 20045, 20045, 20045, 20015, 20042, 20016,
      20042, 20042, 20045, 20045, 20045, 20045, 20045, 20001, 20002, 20002,
      20002, 20045, 20042, 20042, 20045, 20030, 20016, 20016, 20016, 20030,
      20016, 20030, 20016, 20030, 20042, 20042,
    ],
    [
      20042, 20029, 20014, 20002, 20001, 20042, 20042, 20015, 20045, 20002,
      20045, 20016, 20016, 20016, 20015, 20016, 20016, 20042, 20042, 20042,
      20015, 20042, 20042, 20042, 20015, 20042, 20016, 20045, 20002, 20002,
      20045, 20042, 20042, 20016, 20030, 20016, 20016, 20030, 20016, 20016,
      20016, 20016, 20016, 20016, 20042, 20042,
    ],
    [
      20015, 20029, 20002, 20002, 20015, 20042, 20042, 20042, 20045, 20002,
      20045, 20042, 20015, 20015, 20015, 20015, 20042, 20042, 20015, 20015,
      20000, 20015, 20015, 20015, 20015, 20015, 20016, 20016, 20045, 20045,
      20045, 20042, 20042, 20042, 20016, 20030, 20016, 20016, 20016, 20030,
      20016, 20030, 20016, 20030, 20042, 20042,
    ],
  ],
  bgmap: [],
  fgmap: [],
};
main.floors.MT4 = {
  floorId: "MT4",
  title: "主塔 4 层",
  name: "4",
  width: 20,
  height: 27,
  canFlyTo: true,
  canUseQuickShop: true,
  cannotViewMap: false,
  images: [],
  ratio: 1,
  defaultGround: "X20002",
  firstArrive: [],
  eachArrive: [],
  parallelDo: "",
  events: {
    "12,19": ["通往京城"],
    "10,24": ["通往野狼谷"],
    "13,22": [
      {
        type: "switch",
        condition: "flag:child",
        caseList: [
          {
            case: "0",
            action: [
              "\t[老人,role4.png]我和小孙子出来采药，突然一个野狼怪出现，抓了我的孙子向南跑了。请壮士搭救搭救我的孙子吧！",
              { type: "setValue", name: "flag:child", value: "1" },
            ],
          },
          {
            case: "3",
            action: [
              "\t[老人,role4.png]壮士太谢谢你了！没什么可以报答的，我这里有一支千年雪莲，你带上吧，或许对你有些用处，祝你一路顺风！",
              { type: "hide", loc: [[13, 21]], remove: true },
              { type: "hide", loc: [[13, 22]], remove: true },
              { type: "setValue", name: "flag:child", value: "4" },
            ],
          },
          {
            case: "default",
            action: [
              "\t[老人,role4.png]我和小孙子出来采药，突然一个野狼怪出现，抓了我的孙子向南跑了。请壮士搭救搭救我的孙子吧！",
            ],
          },
        ],
      },
    ],
    "6,8": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(3);\n}",
      },
    ],
    "11,3": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(3);\n}",
      },
    ],
    "15,10": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(3);\n}",
      },
    ],
    "9,17": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(2);\n}",
      },
    ],
    "10,22": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(3);\n}",
      },
    ],
    "0,10": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [{ type: "changeFloor", floorId: "MT1", loc: [17, 14] }],
    },
    "11,26": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [
        {
          type: "if",
          condition: "flag:child",
          true: [
            {
              type: "changeFloor",
              floorId: "MT5",
              loc: [1, 32],
              direction: "right",
            },
          ],
          false: ["\t[hero]那里怎么有个老人在哭，去问问到底发生了什么事。"],
        },
      ],
    },
    "19,20": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [
        {
          type: "switch",
          condition: "flag:child",
          caseList: [
            {
              case: "0",
              action: [
                "\t[hero]那里怎么有个老人在哭，去问问到底发生了什么事。",
              ],
            },
            {
              case: "4",
              action: [{ type: "changeFloor", floorId: "MT6", loc: [1, 35] }],
            },
            {
              case: "default",
              action: [
                "\t[hero]师父总是教导我们要替天行道，我怎么能见死不救呢？",
              ],
            },
          ],
        },
      ],
    },
  },
  changeFloor: {},
  afterBattle: {},
  afterGetItem: {},
  afterOpenDoor: {},
  autoEvent: {},
  cannotMove: {},
  map: [
    [
      20015, 20042, 20042, 20016, 20015, 20029, 20014, 20014, 20014, 20029,
      20014, 20015, 20029, 20029, 20042, 20042, 20015, 20042, 20042, 20016,
    ],
    [
      20016, 20042, 20042, 20042, 20016, 20015, 20029, 20029, 20014, 20014,
      20016, 20029, 20015, 20015, 20015, 20015, 20042, 20015, 20042, 20042,
    ],
    [
      20016, 20042, 20042, 20042, 20015, 20029, 20029, 20002, 20002, 20002,
      20014, 20002, 20016, 20002, 20002, 20002, 20002, 20015, 20016, 20016,
    ],
    [
      20015, 20042, 20042, 20042, 20015, 20029, 20002, 20002, 20002, 20002,
      20002, 20002, 20002, 20002, 20045, 20015, 20002, 20002, 20014, 20016,
    ],
    [
      20015, 20015, 20042, 20042, 20015, 20002, 20002, 20002, 20002, 20002,
      20002, 20002, 20002, 20002, 20045, 20015, 20045, 20002, 20029, 20015,
    ],
    [
      20042, 20042, 20042, 20042, 20000, 20002, 20002, 20002, 20002, 20002,
      20002, 20030, 20015, 20015, 20045, 20016, 20002, 20002, 20014, 20015,
    ],
    [
      20042, 20042, 20042, 20042, 20016, 20002, 20002, 20002, 20002, 20002,
      20002, 20030, 20045, 20045, 20016, 20015, 20002, 20045, 20002, 20002,
    ],
    [
      20015, 20016, 20015, 20015, 20000, 20002, 20002, 20002, 20002, 20002,
      20002, 20030, 20045, 20042, 20015, 20002, 20002, 20045, 20015, 20015,
    ],
    [
      20014, 20015, 20014, 20015, 20002, 20002, 20002, 20002, 20002, 20002,
      20002, 20030, 20045, 20042, 20045, 20045, 20002, 20045, 20016, 20016,
    ],
    [
      20000, 20000, 20001, 20002, 20002, 20002, 20002, 20002, 20002, 20002,
      20014, 20030, 20045, 20042, 20015, 20015, 20002, 20045, 20015, 20015,
    ],
    [
      20002, 20002, 20002, 20002, 20002, 20002, 20002, 20014, 20000, 20029,
      20014, 20030, 20045, 20042, 20045, 20002, 20002, 20045, 20015, 20016,
    ],
    [
      20000, 20015, 20000, 20015, 20015, 20015, 20000, 20030, 20029, 20014,
      20030, 20030, 20045, 20042, 20045, 20002, 20045, 20045, 20016, 20015,
    ],
    [
      20015, 20016, 20015, 20045, 20015, 20045, 20030, 20045, 20030, 20030,
      20045, 20045, 20045, 20015, 20015, 20002, 20045, 20016, 20015, 20016,
    ],
    [
      20045, 20045, 20045, 20045, 20045, 20045, 20045, 20015, 20045, 20045,
      20045, 20042, 20042, 20042, 20015, 20002, 20045, 20015, 20015, 20015,
    ],
    [
      20042, 20015, 20042, 20042, 20042, 20015, 20015, 20015, 20015, 20045,
      20042, 20042, 20002, 20002, 20002, 20002, 20045, 20016, 20016, 20016,
    ],
    [
      20015, 20015, 20015, 20042, 20042, 20042, 20015, 20042, 20042, 20045,
      20002, 20002, 20002, 20045, 20045, 20045, 20045, 20015, 20016, 20016,
    ],
    [
      20015, 20042, 20042, 20042, 20042, 20042, 20016, 20015, 20014, 20002,
      20002, 20015, 20045, 20045, 20016, 20016, 20015, 20016, 20015, 20016,
    ],
    [
      20042, 20042, 20042, 20015, 20042, 20042, 20015, 20016, 20016, 20002,
      20015, 20015, 20015, 20016, 20015, 20016, 20016, 20015, 20016, 20015,
    ],
    [
      20042, 20015, 20015, 20015, 20015, 20042, 20016, 20016, 20016, 20002,
      20015, 20002, 20014, 20015, 20016, 20015, 20015, 20015, 20015, 20015,
    ],
    [
      20042, 20042, 20015, 20042, 20042, 20042, 20016, 20015, 20016, 20002,
      20002, 20002, 20031, 20014, 20015, 20015, 20015, 20015, 20015, 20016,
    ],
    [
      20042, 20042, 20042, 20042, 20015, 20042, 20015, 20016, 20045, 20002,
      20002, 20002, 20002, 20002, 20002, 20002, 20002, 20002, 20002, 20002,
    ],
    [
      20042, 20042, 20015, 20015, 20015, 20015, 20016, 20015, 20045, 20045,
      20002, 20002, 20002, 20064, 20072, 20045, 20015, 20015, 20016, 20016,
    ],
    [
      20042, 20042, 20042, 20015, 20042, 20042, 20015, 20015, 20015, 20016,
      20016, 20002, 20002, 20078, 20072, 20015, 20015, 20015, 20015, 20016,
    ],
    [
      20042, 20042, 20015, 20042, 20042, 20042, 20016, 20015, 20016, 20016,
      20016, 20002, 20002, 20014, 20015, 20045, 20015, 20016, 20016, 20016,
    ],
    [
      20015, 20015, 20015, 20015, 20042, 20042, 20015, 20016, 20015, 20029,
      20031, 20002, 20002, 20029, 20045, 20015, 20016, 20042, 20042, 20042,
    ],
    [
      20042, 20015, 20042, 20015, 20042, 20016, 20015, 20015, 20029, 20029,
      20016, 20002, 20045, 20045, 20015, 20015, 20015, 20042, 20042, 20042,
    ],
    [
      20016, 20015, 20015, 20015, 20015, 20015, 20015, 20015, 20015, 20016,
      20015, 20002, 20015, 20042, 20016, 20016, 20016, 20042, 20042, 20042,
    ],
  ],
  bgmap: [],
  fgmap: [],
};
main.floors.MT5 = {
  floorId: "MT5",
  title: "主塔 5 层",
  name: "5",
  width: 44,
  height: 50,
  canFlyTo: true,
  canUseQuickShop: true,
  cannotViewMap: false,
  cannotMoveDirectly: false,
  images: [],
  ratio: 1,
  defaultGround: "X20002",
  firstArrive: [],
  eachArrive: [],
  parallelDo: "",
  events: {
    "5,35": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(4);\n}",
      },
    ],
    "3,43": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(4);\n}",
      },
    ],
    "19,42": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(4);\n}",
      },
    ],
    "29,44": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(3);\n}",
      },
    ],
    "35,28": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(4);\n}",
      },
    ],
    "18,27": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(4);\n}",
      },
    ],
    "18,18": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(3);\n}",
      },
    ],
    "25,4": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(3);\n}",
      },
    ],
    "29,13": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(4);\n}",
      },
    ],
    "27,13": [
      {
        type: "if",
        condition: "flag:pot2",
        true: ["已经检查过了啊！！！"],
        false: [
          {
            type: "setValue",
            name: "item:sword3",
            value: "(item:sword3+1)",
            norefresh: true,
          },
          "得到1个水灵剑",
          { type: "setValue", name: "flag:pot2", value: "1", norefresh: true },
        ],
      },
    ],
    "9,13": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(4);\n}",
      },
    ],
    "6,19": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(4);\n}",
      },
    ],
    "12,31": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(4);\n}",
      },
    ],
    "27,27": [
      "生命恢复",
      { type: "setValue", name: "status:hp", value: "status:hpmax" },
    ],
    "28,26": [
      "生命恢复",
      { type: "setValue", name: "status:hp", value: "status:hpmax" },
    ],
    "36,45": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(4,1);\n}",
      },
    ],
    "40,22": [
      {
        type: "if",
        condition: "(flag:child==1)",
        true: [
          "\t[hero]小弟弟别怕，我是来救你的。",
          "\t[小孩,role5.png]大哥哥小心，背后有妖怪！！",
          {
            type: "function",
            async: true,
            function: "function(){\ncore.battleStart(7, 1);\n}",
          },
        ],
        false: [
          "\t[hero]小弟弟别害怕，我背你去找爷爷吧。",
          { type: "setValue", name: "flag:child", value: "3" },
          { type: "hide", loc: [[40, 21]], remove: true },
          { type: "hide", loc: [[40, 22]], remove: true },
        ],
      },
    ],
    "0,31": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [
        { type: "changeFloor", floorId: "MT4", loc: [11, 25], direction: "up" },
      ],
    },
    "0,32": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [
        { type: "changeFloor", floorId: "MT4", loc: [11, 25], direction: "up" },
      ],
    },
  },
  changeFloor: {},
  afterBattle: {},
  afterGetItem: {},
  afterOpenDoor: {},
  autoEvent: {},
  cannotMove: {},
  map: [
    [
      20016, 20015, 20001, 20002, 20002, 20002, 20001, 20002, 20029, 20014,
      20015, 20015, 20016, 20042, 20042, 20042, 20042, 20016, 20015, 20015,
      20015, 20015, 20016, 20042, 20042, 20042, 20042, 20042, 20042, 20016,
      20016, 20016, 20016, 20015, 20016, 20015, 20042, 20025, 20025, 20025,
      20025, 20025, 20025, 20025,
    ],
    [
      20016, 20015, 20029, 20002, 20002, 20001, 20001, 20002, 20000, 20014,
      20016, 20016, 20016, 20042, 20042, 20042, 20042, 20042, 20016, 20015,
      20016, 20016, 20042, 20042, 20042, 20042, 20042, 20042, 20015, 20016,
      20016, 20016, 20016, 20016, 20015, 20016, 20016, 20025, 20025, 20025,
      20025, 20025, 20025, 20025,
    ],
    [
      20016, 20015, 20029, 20029, 20002, 20002, 20002, 20002, 20000, 20015,
      20015, 20015, 20016, 20016, 20016, 20042, 20042, 20042, 20042, 20042,
      20042, 20042, 20042, 20042, 20015, 20042, 20015, 20015, 20000, 20029,
      20029, 20015, 20016, 20015, 20015, 20015, 20015, 20025, 20025, 20025,
      20025, 20025, 20025, 20025,
    ],
    [
      20016, 20015, 20015, 20002, 20028, 20001, 20002, 20029, 20015, 20015,
      20016, 20015, 20016, 20016, 20016, 20015, 20042, 20042, 20042, 20042,
      20015, 20045, 20015, 20015, 20029, 20014, 20014, 20000, 20000, 20015,
      20029, 20029, 20029, 20015, 20016, 20015, 20016, 20025, 20025, 20025,
      20025, 20025, 20025, 20025,
    ],
    [
      20016, 20016, 20015, 20015, 20028, 20028, 20015, 20029, 20016, 20016,
      20015, 20016, 20016, 20015, 20016, 20016, 20015, 20042, 20042, 20015,
      20015, 20014, 20014, 20029, 20000, 20002, 20002, 20002, 20002, 20000,
      20029, 20029, 20029, 20016, 20016, 20016, 20015, 20025, 20025, 20025,
      20025, 20025, 20025, 20025,
    ],
    [
      20016, 20016, 20016, 20015, 20028, 20028, 20015, 20029, 20016, 20015,
      20015, 20015, 20016, 20016, 20015, 20015, 20014, 20014, 20015, 20000,
      20000, 20014, 20002, 20002, 20001, 20002, 20002, 20002, 20002, 20002,
      20002, 20029, 20029, 20015, 20016, 20015, 20015, 20025, 20025, 20025,
      20025, 20025, 20025, 20025,
    ],
    [
      20016, 20016, 20016, 20016, 20015, 20015, 20016, 20029, 20016, 20015,
      20015, 20016, 20016, 20016, 20015, 20014, 20029, 20000, 20000, 20000,
      20002, 20002, 20001, 20002, 20000, 20000, 20025, 20025, 20014, 20002,
      20001, 20002, 20029, 20029, 20015, 20016, 20016, 20025, 20025, 20025,
      20025, 20025, 20025, 20025,
    ],
    [
      20016, 20015, 20016, 20015, 20016, 20016, 20029, 20016, 20015, 20016,
      20016, 20016, 20015, 20016, 20016, 20015, 20000, 20001, 20002, 20002,
      20001, 20002, 20002, 20000, 20000, 20025, 20025, 20015, 20014, 20014,
      20002, 20002, 20002, 20029, 20016, 20016, 20016, 20025, 20025, 20025,
      20025, 20025, 20025, 20025,
    ],
    [
      20015, 20015, 20016, 20015, 20015, 20029, 20015, 20015, 20015, 20015,
      20016, 20016, 20015, 20029, 20015, 20014, 20014, 20002, 20002, 20000,
      20025, 20000, 20000, 20000, 20016, 20015, 20015, 20016, 20000, 20014,
      20002, 20002, 20002, 20014, 20015, 20016, 20015, 20025, 20025, 20025,
      20025, 20025, 20025, 20025,
    ],
    [
      20016, 20015, 20016, 20016, 20016, 20016, 20016, 20016, 20016, 20016,
      20016, 20015, 20015, 20000, 20029, 20029, 20002, 20002, 20029, 20000,
      20025, 20025, 20025, 20016, 20015, 20016, 20015, 20015, 20016, 20014,
      20000, 20001, 20002, 20015, 20016, 20016, 20016, 20025, 20025, 20025,
      20025, 20025, 20025, 20025,
    ],
    [
      20016, 20016, 20016, 20016, 20015, 20016, 20015, 20016, 20016, 20016,
      20016, 20015, 20014, 20029, 20002, 20002, 20002, 20014, 20042, 20025,
      20025, 20025, 20042, 20015, 20015, 20015, 20015, 20016, 20015, 20016,
      20000, 20002, 20002, 20000, 20015, 20016, 20015, 20025, 20025, 20025,
      20025, 20025, 20025, 20025,
    ],
    [
      20016, 20016, 20016, 20015, 20016, 20016, 20016, 20015, 20015, 20015,
      20015, 20014, 20014, 20002, 20002, 20001, 20029, 20014, 20042, 20025,
      20025, 20042, 20042, 20016, 20015, 20015, 20016, 20029, 20029, 20029,
      20014, 20002, 20001, 20000, 20016, 20015, 20016, 20025, 20025, 20025,
      20025, 20025, 20025, 20025,
    ],
    [
      20016, 20015, 20016, 20016, 20015, 20016, 20015, 20015, 20029, 20029,
      20029, 20014, 20002, 20001, 20002, 20014, 20014, 20015, 20042, 20025,
      20025, 20016, 20016, 20015, 20016, 20016, 20029, 20029, 20029, 20029,
      20002, 20002, 20002, 20000, 20015, 20015, 20016, 20025, 20025, 20025,
      20025, 20025, 20025, 20025,
    ],
    [
      20016, 20016, 20016, 20015, 20015, 20015, 20014, 20001, 20002, 20002,
      20001, 20002, 20002, 20002, 20002, 20002, 20014, 20042, 20025, 20025,
      20042, 20016, 20015, 20016, 20015, 20016, 20029, 20071, 20002, 20002,
      20002, 20000, 20015, 20016, 20015, 20016, 20015, 20025, 20025, 20025,
      20025, 20025, 20025, 20025,
    ],
    [
      20015, 20016, 20015, 20014, 20014, 20029, 20001, 20002, 20002, 20002,
      20002, 20001, 20001, 20002, 20002, 20001, 20002, 20001, 20042, 20042,
      20015, 20016, 20016, 20015, 20016, 20016, 20029, 20029, 20002, 20002,
      20001, 20000, 20015, 20016, 20016, 20015, 20016, 20016, 20016, 20016,
      20015, 20015, 20015, 20015,
    ],
    [
      20016, 20015, 20015, 20029, 20002, 20002, 20002, 20029, 20029, 20015,
      20042, 20015, 20015, 20015, 20002, 20002, 20002, 20002, 20014, 20014,
      20016, 20016, 20015, 20015, 20015, 20016, 20016, 20016, 20029, 20014,
      20016, 20016, 20016, 20016, 20015, 20015, 20015, 20015, 20015, 20016,
      20016, 20042, 20015, 20042,
    ],
    [
      20016, 20015, 20029, 20002, 20002, 20002, 20014, 20014, 20015, 20025,
      20042, 20025, 20025, 20015, 20025, 20015, 20002, 20002, 20002, 20014,
      20016, 20016, 20015, 20042, 20042, 20042, 20015, 20015, 20042, 20042,
      20042, 20042, 20015, 20042, 20042, 20015, 20015, 20015, 20042, 20015,
      20042, 20015, 20042, 20028,
    ],
    [
      20016, 20015, 20029, 20001, 20002, 20002, 20014, 20015, 20042, 20025,
      20042, 20042, 20042, 20042, 20016, 20042, 20001, 20002, 20001, 20014,
      20042, 20015, 20042, 20015, 20042, 20015, 20016, 20042, 20042, 20042,
      20042, 20015, 20015, 20015, 20015, 20014, 20029, 20014, 20015, 20042,
      20015, 20042, 20015, 20028,
    ],
    [
      20015, 20015, 20014, 20001, 20001, 20002, 20002, 20014, 20042, 20025,
      20025, 20042, 20025, 20025, 20025, 20015, 20029, 20002, 20002, 20002,
      20014, 20016, 20015, 20016, 20042, 20042, 20016, 20042, 20015, 20016,
      20015, 20042, 20014, 20001, 20014, 20014, 20001, 20029, 20014, 20001,
      20042, 20015, 20028, 20028,
    ],
    [
      20015, 20015, 20015, 20014, 20002, 20002, 20002, 20014, 20042, 20025,
      20025, 20042, 20025, 20025, 20042, 20042, 20029, 20029, 20002, 20002,
      20014, 20015, 20015, 20015, 20015, 20042, 20042, 20015, 20016, 20015,
      20042, 20015, 20001, 20001, 20002, 20002, 20002, 20002, 20002, 20014,
      20001, 20016, 20028, 20028,
    ],
    [
      20016, 20016, 20016, 20015, 20014, 20002, 20002, 20001, 20015, 20042,
      20025, 20042, 20042, 20042, 20025, 20025, 20015, 20029, 20002, 20002,
      20001, 20029, 20029, 20015, 20042, 20015, 20042, 20042, 20015, 20016,
      20015, 20014, 20002, 20002, 20002, 20002, 20002, 20002, 20002, 20001,
      20001, 20002, 20016, 20028,
    ],
    [
      20025, 20015, 20015, 20015, 20014, 20002, 20002, 20001, 20015, 20042,
      20025, 20025, 20042, 20025, 20016, 20016, 20015, 20029, 20002, 20002,
      20002, 20002, 20029, 20029, 20015, 20016, 20016, 20015, 20015, 20015,
      20015, 20014, 20002, 20002, 20002, 20002, 20015, 20002, 20002, 20002,
      20066, 20014, 20016, 20028,
    ],
    [
      20042, 20025, 20025, 20025, 20015, 20015, 20002, 20029, 20029, 20042,
      20025, 20025, 20042, 20025, 20025, 20025, 20015, 20015, 20029, 20029,
      20029, 20001, 20029, 20015, 20015, 20015, 20042, 20042, 20042, 20042,
      20015, 20042, 20014, 20001, 20002, 20014, 20015, 20002, 20002, 20014,
      20080, 20016, 20016, 20016,
    ],
    [
      20042, 20025, 20016, 20025, 20016, 20015, 20002, 20002, 20029, 20029,
      20042, 20025, 20015, 20042, 20025, 20042, 20042, 20016, 20015, 20015,
      20029, 20002, 20001, 20015, 20042, 20015, 20042, 20015, 20042, 20042,
      20042, 20015, 20042, 20001, 20002, 20002, 20042, 20015, 20015, 20002,
      20014, 20016, 20042, 20042,
    ],
    [
      20025, 20016, 20016, 20016, 20025, 20025, 20002, 20001, 20029, 20042,
      20025, 20025, 20025, 20042, 20042, 20025, 20042, 20015, 20016, 20015,
      20014, 20002, 20002, 20016, 20015, 20015, 20016, 20016, 20015, 20042,
      20042, 20042, 20015, 20015, 20002, 20002, 20001, 20042, 20042, 20015,
      20016, 20016, 20015, 20042,
    ],
    [
      20016, 20015, 20016, 20016, 20015, 20025, 20001, 20002, 20029, 20025,
      20025, 20042, 20016, 20025, 20025, 20025, 20025, 20015, 20014, 20002,
      20001, 20002, 20001, 20015, 20015, 20016, 20015, 20016, 20016, 20016,
      20016, 20042, 20042, 20015, 20029, 20002, 20002, 20015, 20015, 20016,
      20015, 20015, 20042, 20042,
    ],
    [
      20015, 20016, 20015, 20015, 20016, 20015, 20001, 20002, 20002, 20042,
      20042, 20025, 20025, 20015, 20025, 20025, 20015, 20014, 20002, 20001,
      20002, 20002, 20014, 20042, 20042, 20015, 20028, 20028, 20028, 20015,
      20042, 20042, 20042, 20042, 20015, 20002, 20002, 20029, 20015, 20016,
      20016, 20016, 20015, 20042,
    ],
    [
      20016, 20016, 20015, 20016, 20015, 20014, 20002, 20002, 20002, 20016,
      20042, 20025, 20025, 20025, 20025, 20025, 20042, 20015, 20002, 20002,
      20002, 20029, 20029, 20014, 20015, 20016, 20015, 20028, 20002, 20029,
      20029, 20029, 20042, 20015, 20014, 20001, 20002, 20029, 20015, 20015,
      20016, 20042, 20042, 20015,
    ],
    [
      20015, 20016, 20016, 20016, 20015, 20002, 20002, 20002, 20002, 20002,
      20042, 20015, 20015, 20025, 20042, 20025, 20015, 20015, 20001, 20002,
      20014, 20029, 20029, 20014, 20029, 20015, 20016, 20002, 20002, 20002,
      20029, 20029, 20015, 20014, 20002, 20002, 20002, 20015, 20015, 20016,
      20015, 20042, 20015, 20016,
    ],
    [
      20015, 20015, 20016, 20015, 20014, 20002, 20001, 20002, 20002, 20002,
      20001, 20002, 20001, 20015, 20025, 20025, 20042, 20015, 20014, 20002,
      20002, 20002, 20029, 20029, 20015, 20015, 20015, 20002, 20029, 20001,
      20002, 20029, 20042, 20015, 20002, 20002, 20002, 20015, 20015, 20042,
      20042, 20042, 20016, 20016,
    ],
    [
      20015, 20015, 20015, 20015, 20029, 20002, 20001, 20015, 20029, 20029,
      20029, 20002, 20002, 20001, 20042, 20025, 20025, 20042, 20015, 20015,
      20002, 20001, 20002, 20029, 20015, 20042, 20015, 20029, 20029, 20002,
      20002, 20029, 20042, 20015, 20014, 20001, 20002, 20016, 20015, 20015,
      20042, 20042, 20016, 20016,
    ],
    [
      20002, 20002, 20002, 20001, 20002, 20002, 20002, 20015, 20042, 20015,
      20029, 20002, 20002, 20002, 20042, 20025, 20025, 20042, 20042, 20042,
      20015, 20002, 20002, 20014, 20042, 20015, 20042, 20015, 20029, 20002,
      20002, 20014, 20015, 20042, 20015, 20002, 20002, 20042, 20015, 20015,
      20042, 20042, 20015, 20016,
    ],
    [
      20002, 20001, 20002, 20002, 20002, 20002, 20015, 20025, 20025, 20042,
      20029, 20029, 20002, 20002, 20042, 20025, 20025, 20025, 20015, 20015,
      20002, 20002, 20002, 20014, 20015, 20015, 20015, 20015, 20029, 20002,
      20014, 20014, 20042, 20015, 20015, 20002, 20002, 20015, 20042, 20042,
      20042, 20042, 20016, 20015,
    ],
    [
      20014, 20014, 20002, 20002, 20001, 20002, 20042, 20016, 20025, 20025,
      20042, 20029, 20002, 20015, 20025, 20025, 20015, 20015, 20014, 20001,
      20002, 20002, 20001, 20002, 20002, 20029, 20029, 20029, 20002, 20001,
      20014, 20015, 20016, 20015, 20014, 20002, 20001, 20042, 20015, 20042,
      20015, 20015, 20015, 20016,
    ],
    [
      20015, 20015, 20014, 20014, 20002, 20002, 20015, 20025, 20025, 20025,
      20025, 20025, 20070, 20042, 20025, 20015, 20014, 20002, 20002, 20002,
      20002, 20002, 20002, 20002, 20002, 20002, 20002, 20002, 20002, 20002,
      20002, 20016, 20015, 20016, 20015, 20002, 20002, 20015, 20015, 20042,
      20042, 20015, 20016, 20015,
    ],
    [
      20016, 20015, 20015, 20014, 20002, 20002, 20001, 20042, 20025, 20042,
      20025, 20025, 20025, 20025, 20042, 20015, 20029, 20001, 20002, 20002,
      20015, 20015, 20029, 20014, 20016, 20002, 20002, 20002, 20002, 20002,
      20002, 20029, 20042, 20015, 20002, 20001, 20002, 20015, 20042, 20042,
      20042, 20015, 20042, 20015,
    ],
    [
      20016, 20015, 20029, 20029, 20002, 20001, 20002, 20015, 20025, 20016,
      20025, 20025, 20042, 20025, 20042, 20042, 20015, 20002, 20002, 20002,
      20014, 20015, 20015, 20016, 20015, 20015, 20015, 20014, 20014, 20001,
      20002, 20002, 20001, 20002, 20002, 20002, 20014, 20029, 20015, 20016,
      20042, 20042, 20016, 20028,
    ],
    [
      20015, 20015, 20029, 20002, 20002, 20002, 20001, 20042, 20025, 20025,
      20042, 20025, 20025, 20025, 20025, 20015, 20014, 20002, 20002, 20001,
      20029, 20029, 20015, 20015, 20016, 20016, 20015, 20015, 20015, 20015,
      20042, 20002, 20002, 20002, 20002, 20014, 20029, 20015, 20015, 20016,
      20042, 20042, 20015, 20028,
    ],
    [
      20016, 20015, 20014, 20002, 20002, 20002, 20002, 20015, 20025, 20025,
      20025, 20025, 20042, 20025, 20025, 20025, 20015, 20002, 20002, 20001,
      20029, 20014, 20015, 20016, 20015, 20016, 20042, 20042, 20042, 20015,
      20002, 20002, 20014, 20015, 20015, 20029, 20029, 20029, 20015, 20016,
      20015, 20015, 20016, 20028,
    ],
    [
      20016, 20016, 20015, 20002, 20002, 20015, 20015, 20025, 20025, 20042,
      20025, 20025, 20025, 20016, 20025, 20025, 20042, 20015, 20002, 20002,
      20014, 20015, 20016, 20016, 20016, 20015, 20042, 20042, 20015, 20016,
      20002, 20002, 20014, 20015, 20016, 20015, 20029, 20015, 20016, 20015,
      20016, 20016, 20028, 20028,
    ],
    [
      20025, 20025, 20015, 20002, 20002, 20014, 20016, 20042, 20025, 20025,
      20016, 20025, 20025, 20042, 20025, 20025, 20025, 20015, 20014, 20002,
      20014, 20014, 20015, 20016, 20015, 20016, 20042, 20042, 20015, 20002,
      20002, 20014, 20015, 20015, 20016, 20016, 20015, 20015, 20016, 20016,
      20015, 20016, 20028, 20028,
    ],
    [
      20016, 20025, 20015, 20002, 20001, 20015, 20025, 20016, 20025, 20025,
      20042, 20025, 20025, 20025, 20025, 20025, 20025, 20042, 20015, 20001,
      20002, 20029, 20015, 20016, 20016, 20015, 20015, 20016, 20014, 20002,
      20001, 20015, 20016, 20016, 20016, 20015, 20016, 20016, 20016, 20016,
      20016, 20016, 20016, 20028,
    ],
    [
      20016, 20016, 20025, 20002, 20001, 20015, 20015, 20042, 20015, 20042,
      20025, 20025, 20025, 20015, 20042, 20015, 20042, 20015, 20001, 20002,
      20002, 20029, 20029, 20015, 20016, 20016, 20016, 20015, 20002, 20002,
      20015, 20015, 20016, 20016, 20016, 20015, 20016, 20016, 20016, 20016,
      20016, 20015, 20016, 20028,
    ],
    [
      20016, 20016, 20015, 20001, 20002, 20002, 20002, 20002, 20014, 20014,
      20015, 20025, 20015, 20014, 20002, 20002, 20001, 20002, 20001, 20002,
      20002, 20029, 20029, 20015, 20016, 20016, 20016, 20014, 20001, 20002,
      20029, 20016, 20015, 20016, 20015, 20016, 20015, 20015, 20016, 20016,
      20015, 20016, 20016, 20016,
    ],
    [
      20016, 20016, 20015, 20002, 20001, 20002, 20002, 20001, 20002, 20002,
      20002, 20002, 20001, 20002, 20002, 20002, 20002, 20002, 20002, 20002,
      20002, 20001, 20029, 20015, 20016, 20016, 20016, 20015, 20014, 20002,
      20002, 20014, 20014, 20015, 20015, 20016, 20016, 20016, 20015, 20016,
      20016, 20015, 20016, 20028,
    ],
    [
      20016, 20015, 20016, 20002, 20002, 20029, 20029, 20001, 20002, 20002,
      20001, 20002, 20002, 20002, 20001, 20002, 20002, 20002, 20002, 20002,
      20014, 20014, 20014, 20016, 20015, 20016, 20015, 20015, 20014, 20002,
      20001, 20002, 20014, 20014, 20015, 20001, 340, 20016, 20015, 20016, 20016,
      20016, 20016, 20028,
    ],
    [
      20016, 20015, 20016, 20016, 20029, 20014, 20015, 20029, 20002, 20002,
      20002, 20002, 20002, 20002, 20002, 20002, 20001, 20014, 20015, 20014,
      20001, 20001, 20015, 20016, 20016, 20016, 20015, 20014, 20029, 20029,
      20002, 20002, 20002, 20002, 20002, 20002, 20001, 20016, 20016, 20015,
      20016, 20016, 20016, 20028,
    ],
    [
      20016, 20016, 20015, 20015, 20014, 20015, 20015, 20015, 20015, 20029,
      20029, 20015, 20014, 20015, 20015, 20014, 20015, 20015, 20016, 20015,
      20015, 20015, 20016, 20015, 20016, 20016, 20015, 20014, 20014, 20029,
      20029, 20014, 20029, 20029, 20029, 20001, 20016, 20016, 20015, 20016,
      20016, 20016, 20028, 20028,
    ],
    [
      20016, 20016, 20016, 20016, 20015, 20016, 20016, 20015, 20016, 20016,
      20016, 20015, 20015, 20016, 20016, 20015, 20016, 20015, 20016, 20016,
      20016, 20015, 20016, 20016, 20016, 20016, 20016, 20015, 20016, 20014,
      20015, 20014, 20014, 20016, 20015, 20016, 20016, 20015, 20016, 20016,
      20016, 20016, 20028, 20028,
    ],
    [
      20016, 20016, 20016, 20016, 20016, 20015, 20015, 20016, 20015, 20015,
      20016, 20015, 20016, 20015, 20016, 20016, 20016, 20015, 20016, 20016,
      20016, 20016, 20016, 20016, 20016, 20016, 20016, 20016, 20015, 20015,
      20016, 20015, 20015, 20016, 20016, 20015, 20015, 20016, 20016, 20016,
      20016, 20016, 20028, 20028,
    ],
  ],
  bgmap: [],
  fgmap: [],
};
main.floors.MT6 = {
  floorId: "MT6",
  title: "主塔 6 层",
  name: "6",
  width: 38,
  height: 56,
  canFlyTo: true,
  canUseQuickShop: true,
  cannotViewMap: false,
  cannotMoveDirectly: false,
  images: [],
  ratio: 1,
  defaultGround: "X20002",
  firstArrive: [],
  eachArrive: [],
  parallelDo: "",
  events: {
    "3,24": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(5);\n}",
      },
    ],
    "7,32": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(4);\n}",
      },
    ],
    "11,23": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(5);\n}",
      },
    ],
    "12,34": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(4);\n}",
      },
    ],
    "10,43": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(5);\n}",
      },
    ],
    "2,47": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(6);\n}",
      },
    ],
    "13,51": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(5);\n}",
      },
    ],
    "23,52": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(5);\n}",
      },
    ],
    "36,54": [
      "\t[箱子老怪,role2.png]哪个敢打扰我箱子老怪休息啊，真是找死，接招吧！！！",
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(10);\n}",
      },
    ],
    "32,49": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(6);\n}",
      },
    ],
    "26,33": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(6);\n}",
      },
    ],
    "27,23": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(6);\n}",
      },
    ],
    "20,12": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(5);\n}",
      },
    ],
    "5,7": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(6);\n}",
      },
    ],
    "32,12": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(5);\n}",
      },
    ],
    "29,2": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(6);\n}",
      },
    ],
    "28,2": [
      "生命恢复",
      { type: "setValue", name: "status:hp", value: "status:hpmax" },
    ],
    "27,3": [
      "生命恢复",
      { type: "setValue", name: "status:hp", value: "status:hpmax" },
    ],
    "0,35": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [{ type: "changeFloor", floorId: "MT4", loc: [18, 20] }],
    },
    "37,7": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [{ type: "changeFloor", floorId: "MT7", loc: [1, 4] }],
    },
    "37,8": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [{ type: "changeFloor", floorId: "MT7", loc: [1, 4] }],
    },
  },
  changeFloor: {},
  afterBattle: {},
  afterGetItem: {},
  afterOpenDoor: {},
  autoEvent: {},
  cannotMove: {},
  map: [
    [
      20028, 20016, 20016, 20016, 20015, 20016, 20016, 20016, 20015, 20016,
      20016, 20016, 20015, 20016, 20016, 20016, 20015, 20016, 20016, 20016,
      20015, 20015, 20015, 20016, 20016, 20016, 20016, 20016, 20016, 20015,
      20015, 20015, 20015, 20016, 20016, 20015, 20016, 20016,
    ],
    [
      20028, 20016, 20015, 20015, 20015, 20016, 20015, 20015, 20015, 20015,
      20015, 20015, 20015, 20015, 20015, 20015, 20015, 20015, 20016, 20016,
      20015, 20016, 20016, 20016, 20015, 20016, 20015, 20015, 20015, 20029,
      20016, 20016, 20029, 20015, 20015, 20015, 20015, 20016,
    ],
    [
      20028, 20015, 20015, 20015, 20015, 20015, 20029, 20015, 20014, 20029,
      20014, 20015, 20029, 20016, 20016, 20015, 20016, 20016, 20016, 20016,
      20015, 20016, 20015, 20015, 20016, 20015, 20028, 20028, 20028, 20002,
      20002, 20029, 20029, 20029, 20015, 20016, 20016, 20015,
    ],
    [
      20028, 20015, 20015, 20015, 20016, 20029, 20014, 20002, 20002, 20002,
      20014, 20029, 20029, 20014, 20016, 20015, 20016, 20016, 20015, 20015,
      20015, 20015, 20016, 20015, 20016, 20015, 20028, 20028, 20001, 20002,
      20002, 20002, 20029, 20029, 20029, 20015, 20015, 20015,
    ],
    [
      20028, 20016, 20016, 20016, 20016, 20015, 20002, 20002, 20002, 20002,
      20002, 20002, 20014, 20015, 20015, 20015, 20015, 20016, 20016, 20015,
      20016, 20016, 20016, 20015, 20016, 20029, 20002, 20015, 20015, 20002,
      20002, 20002, 20001, 20002, 20029, 20014, 20015, 20016,
    ],
    [
      20028, 20028, 20016, 20016, 20015, 20016, 20002, 20002, 20002, 20002,
      20002, 20002, 20002, 20014, 20015, 20014, 20016, 20016, 20016, 20015,
      20016, 20015, 20015, 20015, 20015, 20016, 20016, 20029, 20029, 20002,
      20002, 20001, 20001, 20002, 20002, 20002, 20015, 20015,
    ],
    [
      20028, 20028, 20028, 20028, 20016, 20016, 20002, 20002, 20014, 20029,
      20014, 20014, 20002, 20002, 20002, 20002, 20002, 20015, 20015, 20015,
      20015, 20016, 20015, 20015, 20015, 20015, 20015, 20016, 20016, 20029,
      20029, 20002, 20002, 20001, 20002, 20002, 20014, 20015,
    ],
    [
      20016, 20028, 20028, 20028, 20002, 20002, 20002, 20014, 20002, 20015,
      20029, 20014, 20029, 20014, 20002, 20002, 20002, 20014, 20015, 20014,
      20016, 20016, 20015, 20016, 20015, 20016, 20015, 20015, 20015, 20015,
      20029, 20029, 20001, 20002, 20002, 20002, 20002, 20002,
    ],
    [
      20016, 20016, 20028, 20028, 20001, 20014, 20002, 340, 20016, 20016, 20016,
      20015, 20015, 20014, 20029, 20014, 20002, 20002, 20002, 20014, 20015,
      20015, 20015, 20015, 20016, 20016, 20016, 20015, 20016, 20016, 20016,
      20015, 20029, 20002, 20002, 20001, 20002, 20002,
    ],
    [
      20016, 20016, 20028, 20028, 20028, 20028, 20028, 20016, 20015, 20016,
      20015, 20016, 20015, 20016, 20015, 20015, 20029, 20014, 20002, 20002,
      20014, 20015, 20016, 20016, 20015, 20016, 20016, 20016, 20016, 20015,
      20015, 20015, 20015, 20002, 20001, 20002, 20015, 20015,
    ],
    [
      20016, 20015, 20016, 20028, 20028, 20028, 20028, 20028, 20028, 20015,
      20015, 20016, 20016, 20015, 20016, 20016, 20015, 20015, 20014, 20002,
      20002, 20014, 20015, 20015, 20015, 20015, 20016, 20016, 20016, 20016,
      20015, 20029, 20014, 20002, 20002, 20014, 20014, 20028,
    ],
    [
      20042, 20016, 20016, 20016, 20016, 20015, 20016, 20028, 20028, 20028,
      20028, 20028, 20028, 20028, 20028, 20016, 20016, 20015, 20029, 20002,
      20002, 20029, 20014, 20015, 20016, 20016, 20016, 20016, 20015, 20016,
      20029, 20014, 20002, 20001, 20002, 20015, 20028, 20028,
    ],
    [
      20042, 20015, 20016, 20016, 20015, 20016, 20015, 20016, 20016, 20028,
      20028, 20028, 20028, 20028, 20028, 20016, 20016, 20016, 20015, 20002,
      20002, 20014, 20029, 20015, 20016, 20016, 20015, 20015, 20015, 20015,
      20029, 20014, 20002, 20002, 20014, 20015, 20028, 20028,
    ],
    [
      20015, 20042, 20016, 20016, 20016, 20015, 20016, 20016, 20015, 20016,
      20016, 20028, 20028, 20028, 20028, 20028, 20028, 20015, 20014, 20002,
      20002, 20015, 20015, 20015, 20015, 20016, 20016, 20015, 20016, 20015,
      20014, 20002, 20002, 20014, 20015, 20015, 0, 20028,
    ],
    [
      20042, 20042, 20042, 20042, 20015, 20015, 20015, 20015, 20016, 20015,
      20016, 20028, 20028, 20028, 20028, 20028, 20028, 20016, 20015, 20002,
      20002, 20002, 20015, 20014, 20016, 20016, 20016, 20015, 20015, 20015,
      20015, 20001, 20002, 20014, 20014, 20015, 20028, 20028,
    ],
    [
      20042, 20042, 20042, 20042, 20042, 20042, 20015, 20016, 20015, 20016,
      20016, 20028, 20028, 20028, 20028, 20028, 20028, 20028, 20015, 20014,
      20002, 20002, 20002, 20002, 20015, 20015, 20015, 20015, 20015, 20014,
      20001, 20002, 20014, 20015, 20015, 20028, 20028, 20028,
    ],
    [
      20016, 20042, 20015, 20042, 20042, 20042, 20042, 20015, 20015, 20015,
      20016, 20028, 20028, 20028, 20028, 20028, 20028, 20028, 20016, 20015,
      20015, 20014, 20002, 20002, 20002, 20015, 20016, 20015, 20002, 20002,
      20002, 20002, 20015, 20015, 20015, 20028, 20028, 20015,
    ],
    [
      20016, 20016, 20042, 20042, 20042, 20042, 20015, 20016, 20016, 20016,
      20016, 20028, 20028, 20028, 20028, 20028, 20028, 20028, 20016, 20016,
      20015, 20014, 20015, 20002, 20002, 20014, 20001, 20002, 20002, 20002,
      20029, 20014, 20015, 20015, 20016, 20016, 20028, 20028,
    ],
    [
      20016, 20016, 20042, 20015, 20042, 20016, 20016, 20016, 20016, 20015,
      20016, 20016, 20028, 20028, 20028, 20028, 20028, 20028, 20028, 20028,
      20016, 20015, 20029, 20029, 20002, 20002, 20002, 20002, 20014, 20015,
      20015, 20015, 20016, 20016, 20016, 20028, 20028, 20028,
    ],
    [
      20016, 20042, 20015, 20042, 20015, 20015, 20016, 20015, 20015, 20029,
      20015, 20015, 20015, 20016, 20016, 20028, 20028, 20028, 20028, 20028,
      20016, 20015, 20015, 20014, 20002, 20002, 20002, 20014, 20015, 20016,
      20015, 20015, 20016, 20015, 20028, 20028, 20028, 20015,
    ],
    [
      20016, 20042, 20042, 20015, 20029, 20014, 20015, 20029, 20014, 20029,
      20015, 20014, 20015, 20016, 20015, 20028, 20028, 20028, 20028, 20016,
      20015, 20016, 20016, 20015, 20014, 20002, 20002, 20002, 20015, 20015,
      20016, 20028, 20028, 20028, 20028, 20028, 20028, 20028,
    ],
    [
      20042, 20042, 20015, 20029, 20002, 20002, 20002, 20002, 20002, 20002,
      20002, 20002, 20029, 20015, 20028, 20028, 20028, 20028, 20028, 20016,
      20016, 20015, 20015, 20015, 20015, 20002, 20001, 20002, 20001, 20015,
      20015, 20028, 20028, 20028, 20028, 20028, 20028, 20028,
    ],
    [
      20042, 20042, 20002, 20002, 20002, 20002, 20002, 20002, 20002, 20002,
      20002, 20002, 20002, 20014, 20028, 20028, 20028, 20028, 20028, 20028,
      20016, 20015, 20029, 20014, 20002, 20002, 20002, 20002, 20002, 20015,
      20015, 20028, 20028, 20028, 20028, 20028, 20028, 20015,
    ],
    [
      20042, 20015, 20014, 20002, 20002, 20045, 20045, 20002, 20002, 20045,
      20002, 20002, 20002, 20016, 20028, 20028, 20028, 20028, 20028, 20028,
      20016, 20016, 20015, 20015, 20002, 20001, 20002, 20002, 20002, 20015,
      20028, 20028, 20028, 20028, 20028, 20028, 20028, 20028,
    ],
    [
      20015, 20014, 20002, 20002, 20002, 20045, 20016, 20045, 20045, 20045,
      20045, 20002, 20002, 20015, 20016, 20028, 20028, 20028, 20028, 20016,
      20016, 20016, 20015, 20016, 20029, 20002, 20002, 20001, 20002, 20015,
      20028, 20028, 20028, 20028, 20028, 20028, 20028, 20015,
    ],
    [
      20029, 20029, 20002, 20002, 20045, 20045, 20015, 20016, 20015, 20016,
      20045, 20002, 20002, 20016, 20015, 20028, 20028, 20028, 20028, 20016,
      20028, 20028, 20028, 20015, 20014, 20001, 20002, 20002, 20002, 20015,
      20028, 20028, 20028, 20028, 20028, 20028, 20028, 20029,
    ],
    [
      20015, 20014, 20002, 20002, 20045, 20015, 20014, 20015, 20016, 20015,
      20045, 20002, 20002, 20014, 20016, 20028, 20028, 20028, 20028, 20016,
      20016, 20028, 20028, 20015, 20014, 20001, 20002, 20002, 20002, 20015,
      20028, 20028, 20028, 20028, 20028, 20028, 20028, 20015,
    ],
    [
      20015, 20029, 20014, 20002, 20002, 20002, 20014, 20015, 20016, 20016,
      20015, 20045, 20002, 20002, 20029, 20016, 20028, 20028, 20028, 20016,
      20028, 20028, 20016, 20014, 20001, 20002, 20002, 20002, 20015, 20028,
      20028, 20028, 20028, 20028, 20028, 20028, 20028, 20028,
    ],
    [
      20016, 20015, 20045, 20029, 20002, 20002, 20002, 20015, 20016, 20016,
      20016, 20045, 20002, 20002, 20016, 20015, 20028, 20028, 20028, 20028,
      20028, 20016, 20029, 20014, 20002, 20002, 20002, 20015, 20015, 20028,
      20028, 20028, 20028, 20028, 20028, 20028, 20028, 20028,
    ],
    [
      20042, 20016, 20015, 20045, 20014, 20002, 20002, 20002, 20015, 20016,
      20015, 20045, 20002, 20002, 20002, 20016, 20016, 20028, 20028, 20028,
      20016, 20015, 20029, 20014, 20002, 20001, 20015, 20015, 20016, 20028,
      20028, 20028, 20028, 20028, 20028, 20028, 20028, 20016,
    ],
    [
      20042, 20042, 20016, 20045, 20014, 20014, 20002, 20002, 20015, 20016,
      20015, 20045, 20002, 20002, 20002, 20016, 20016, 20028, 20028, 20016,
      20015, 20014, 20029, 20001, 20002, 20002, 20015, 20016, 20016, 20028,
      20028, 20028, 20028, 20028, 20028, 20028, 20028, 20016,
    ],
    [
      20016, 20016, 20015, 20045, 20045, 20045, 20002, 20002, 20002, 20015,
      20016, 20045, 20002, 20002, 20045, 20016, 20015, 20016, 20028, 20028,
      20028, 20016, 20014, 20001, 20002, 20002, 20014, 20015, 20015, 20016,
      20028, 20028, 20028, 20028, 20028, 20028, 20015, 20016,
    ],
    [
      20014, 20016, 20016, 20015, 20015, 20045, 20002, 20002, 20002, 20015,
      20016, 20045, 20002, 20002, 20016, 20015, 20016, 20016, 20028, 20028,
      20015, 20029, 20029, 20001, 20002, 20002, 20014, 20014, 20014, 20015,
      20015, 20016, 20028, 20028, 20015, 20028, 20028, 20015,
    ],
    [
      20029, 20015, 20015, 20015, 20014, 20045, 20002, 20002, 20002, 20015,
      20015, 20045, 20002, 20002, 20014, 20015, 20016, 20016, 20015, 20028,
      20015, 20016, 20014, 20014, 20002, 20001, 20002, 20001, 20002, 20002,
      20002, 20015, 20015, 20016, 20015, 20016, 20028, 20028,
    ],
    [
      20014, 20015, 20014, 20014, 20002, 20002, 20002, 20014, 20045, 20015,
      20016, 20045, 20002, 20002, 20016, 20016, 20042, 20042, 20016, 20028,
      20028, 20016, 20016, 20015, 20015, 20002, 20002, 20002, 20001, 20002,
      20002, 20001, 20014, 20015, 20016, 20016, 20016, 20028,
    ],
    [
      20002, 20002, 20002, 20002, 20002, 20029, 20029, 20045, 20045, 20016,
      20045, 20002, 20002, 20002, 20016, 20042, 20042, 20042, 20016, 20028,
      20028, 20028, 20028, 20042, 20042, 20042, 20002, 20015, 20014, 20002,
      20002, 20002, 20029, 20014, 20015, 20016, 20015, 20028,
    ],
    [
      20015, 20015, 20045, 20045, 20014, 20014, 20045, 20045, 20016, 20015,
      20045, 20002, 20002, 20002, 20015, 20042, 20042, 20028, 20028, 20028,
      20028, 20028, 20028, 20028, 20028, 20042, 20042, 20042, 20042, 20015,
      20001, 20002, 20002, 20002, 20014, 20015, 20028, 20028,
    ],
    [
      20016, 20016, 20016, 20015, 20045, 20045, 20045, 20016, 20015, 20014,
      20045, 20002, 20002, 20045, 20015, 20042, 20042, 20028, 20028, 20028,
      20028, 20028, 20028, 20028, 20028, 20028, 20042, 20042, 20042, 20042,
      20029, 20002, 20001, 20002, 20002, 20015, 20028, 20015,
    ],
    [
      20015, 20015, 20015, 20016, 20015, 20016, 20016, 20016, 20015, 20014,
      20002, 20002, 20002, 20045, 20016, 20042, 20028, 20028, 20028, 20028,
      20028, 20028, 20028, 20028, 20028, 20028, 20042, 20042, 20042, 20015,
      20042, 20014, 20002, 20002, 20002, 20014, 20028, 20028,
    ],
    [
      20015, 20016, 20016, 20015, 20016, 20016, 20016, 20015, 20029, 20002,
      20002, 20002, 20045, 20016, 20016, 20028, 20028, 20028, 20028, 20028,
      20028, 20028, 20028, 20028, 20028, 20028, 20042, 20016, 20042, 20015,
      20015, 20029, 20001, 20002, 20001, 20014, 20015, 20028,
    ],
    [
      20016, 20015, 20015, 20015, 20015, 20016, 20015, 20014, 20029, 20002,
      20002, 20002, 20045, 20016, 20028, 20028, 20028, 20028, 20028, 20028,
      20028, 20028, 20028, 20028, 20028, 20028, 20028, 20016, 20042, 20014,
      20014, 20002, 20002, 20001, 20014, 20015, 20016, 20028,
    ],
    [
      20015, 20016, 20016, 20015, 20016, 20015, 20014, 20015, 20002, 20002,
      20002, 20002, 20045, 20045, 20016, 20042, 20028, 20028, 20028, 20028,
      20028, 20028, 20028, 20028, 20028, 20028, 20028, 20015, 20014, 20029,
      20002, 20002, 20002, 20001, 20015, 20015, 20015, 20028,
    ],
    [
      20015, 20015, 20015, 20016, 20015, 20015, 20014, 20001, 20002, 20002,
      20002, 20002, 20001, 20045, 20016, 20016, 20028, 20028, 20028, 20028,
      20028, 20028, 20028, 20028, 20028, 20028, 20028, 20029, 20014, 20002,
      20002, 20002, 20029, 20029, 20015, 20016, 20015, 20016,
    ],
    [
      20016, 20016, 20016, 20015, 20016, 20016, 20015, 20001, 20002, 20002,
      20002, 20001, 20002, 20045, 20015, 20016, 20016, 20028, 20028, 20028,
      20028, 20028, 20028, 20028, 20028, 20028, 20028, 20028, 20014, 20002,
      20002, 20002, 20029, 20015, 20016, 20015, 20016, 20016,
    ],
    [
      20016, 20016, 20015, 20015, 20015, 20015, 20029, 20014, 20002, 20002,
      20001, 20002, 20002, 20045, 20016, 20016, 20016, 20028, 20028, 20028,
      20028, 20028, 20028, 20028, 20028, 20028, 20028, 20028, 20001, 20002,
      20002, 20002, 20029, 20014, 20015, 20016, 20016, 20016,
    ],
    [
      20015, 20016, 20015, 20015, 20015, 20015, 20014, 20001, 20002, 20002,
      20001, 20002, 20001, 20016, 20016, 20028, 20028, 20028, 20028, 20028,
      20028, 20028, 20028, 20028, 20028, 20028, 20028, 20028, 20016, 20001,
      20002, 20002, 20014, 20029, 20029, 20015, 20015, 20015,
    ],
    [
      20016, 20015, 20016, 20014, 20002, 20001, 20002, 20002, 20002, 20002,
      20002, 20029, 20029, 20016, 20028, 20028, 20028, 20028, 20028, 20028,
      20028, 20028, 20028, 20028, 20028, 20028, 20028, 20028, 20016, 20014,
      20001, 20002, 20029, 20014, 20015, 20016, 20016, 20015,
    ],
    [
      20015, 20015, 20002, 20002, 20001, 20016, 20016, 20002, 20002, 20002,
      20029, 20029, 20029, 20016, 20028, 20028, 20028, 20028, 20028, 20028,
      20028, 20028, 20028, 20028, 20028, 20028, 20028, 20028, 20016, 20014,
      20001, 20002, 20029, 20029, 20014, 20015, 20016, 20028,
    ],
    [
      20015, 20016, 340, 20002, 20016, 20015, 20002, 20002, 20001, 20002, 20002,
      20014, 20029, 20029, 20016, 20028, 20028, 20028, 20028, 20028, 20028,
      20028, 20016, 20042, 20028, 20028, 20028, 20028, 20016, 20015, 20002,
      20002, 20002, 20029, 20015, 20016, 20016, 20028,
    ],
    [
      20002, 20002, 20016, 20015, 20016, 20001, 20002, 20002, 20002, 20001,
      20002, 20002, 20014, 20029, 20029, 20016, 20016, 20016, 20016, 20028,
      20028, 20016, 20016, 20029, 20042, 20042, 20016, 20016, 20014, 20002,
      20002, 20002, 20001, 20029, 20014, 20015, 20016, 20028,
    ],
    [
      20016, 20016, 20016, 20016, 20016, 20015, 20015, 20002, 20001, 20002,
      20002, 20002, 20001, 20002, 20001, 20002, 20016, 20014, 20029, 20014,
      20016, 20029, 20029, 20029, 20016, 20014, 20001, 20002, 20002, 20001,
      20002, 20002, 20001, 20002, 20015, 20015, 20015, 20028,
    ],
    [
      20015, 20016, 20016, 20016, 20015, 20016, 20016, 20015, 20001, 20002,
      20002, 20002, 20001, 20002, 20002, 20001, 20002, 20001, 20002, 20014,
      20029, 20029, 20014, 20002, 20001, 20002, 20001, 20001, 20001, 20002,
      20002, 20001, 20002, 20002, 20015, 20016, 20015, 20016,
    ],
    [
      20016, 20016, 20015, 20015, 20015, 20015, 20015, 20014, 20014, 20002,
      20001, 20002, 20002, 20002, 20002, 20002, 20002, 20002, 20002, 20001,
      20002, 20002, 20002, 20002, 20002, 20015, 20015, 20042, 20042, 20029,
      20029, 20029, 20029, 20001, 20016, 20015, 20016, 20016,
    ],
    [
      20042, 20042, 20042, 20015, 20042, 20042, 20042, 20015, 20029, 20014,
      20029, 20015, 20042, 20014, 20002, 20002, 20002, 20002, 20002, 20001,
      20001, 20001, 20002, 20002, 20015, 20015, 20015, 20015, 20015, 20042,
      20042, 20015, 20029, 20001, 20001, 20002, 20002, 20016,
    ],
    [
      20042, 20042, 20042, 20042, 20042, 20042, 20042, 20015, 20015, 20015,
      20015, 20015, 20015, 20042, 20015, 20014, 20002, 20014, 20015, 20014,
      20029, 20014, 20029, 20015, 20016, 20015, 20042, 20042, 20042, 20015,
      20015, 20042, 20015, 20016, 20014, 20002, 20070, 20016,
    ],
    [
      20042, 20042, 20042, 20016, 20016, 20015, 20042, 20042, 20015, 20042,
      20015, 20042, 20015, 20015, 20015, 20015, 20015, 20015, 20015, 20015,
      20016, 20015, 20015, 20015, 20015, 20016, 20042, 20042, 20042, 20042,
      20042, 20042, 20015, 20042, 20016, 20016, 20016, 20015,
    ],
  ],
  bgmap: [],
  fgmap: [],
};
main.floors.MT7 = {
  floorId: "MT7",
  title: "主塔 7 层",
  name: "7",
  width: 20,
  height: 59,
  canFlyTo: true,
  canUseQuickShop: true,
  cannotViewMap: false,
  cannotMoveDirectly: false,
  images: [],
  ratio: 1,
  defaultGround: "X20002",
  firstArrive: [],
  eachArrive: [],
  parallelDo: "",
  events: {
    "16,4": [
      {
        type: "if",
        condition: "flag:pot3",
        true: ["已经检查过了啊！！！"],
        false: [
          {
            type: "setValue",
            name: "item:sword4",
            value: "(item:sword4+1)",
            norefresh: true,
          },
          "得到一个火云刀。",
          { type: "setValue", name: "flag:pot3", value: "1", norefresh: true },
        ],
      },
    ],
    "6,33": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(5);\n}",
      },
    ],
    "2,21": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(5);\n}",
      },
    ],
    "6,8": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(6);\n}",
      },
    ],
    "13,18": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(6);\n}",
      },
    ],
    "16,35": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(5);\n}",
      },
    ],
    "0,3": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [{ type: "changeFloor", floorId: "MT6", loc: [36, 8] }],
    },
    "0,4": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [{ type: "changeFloor", floorId: "MT6", loc: [36, 8] }],
    },
    "19,37": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [
        {
          type: "changeFloor",
          floorId: "MT8",
          loc: [17, 1],
          direction: "down",
        },
      ],
    },
    "19,38": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [
        {
          type: "changeFloor",
          floorId: "MT8",
          loc: [17, 1],
          direction: "down",
        },
      ],
    },
  },
  changeFloor: {},
  afterBattle: {},
  afterGetItem: {},
  afterOpenDoor: {},
  autoEvent: {},
  cannotMove: {},
  map: [
    [
      20016, 20015, 20016, 20016, 20016, 20015, 20016, 20016, 20016, 20016,
      20016, 20015, 20015, 20015, 20015, 20042, 20015, 20042, 20042, 20042,
    ],
    [
      20015, 20015, 20015, 20015, 20015, 20015, 20015, 20016, 20016, 20016,
      20016, 20042, 20015, 20042, 20016, 20016, 20016, 20016, 20015, 20015,
    ],
    [
      20015, 20029, 20029, 20029, 20015, 20016, 20016, 20016, 20016, 20016,
      20015, 20016, 20015, 20042, 20015, 20016, 20016, 20016, 20016, 20015,
    ],
    [
      20002, 20001, 20002, 20029, 20029, 20029, 20016, 20015, 20015, 20015,
      20015, 20015, 20001, 20015, 20001, 20015, 20015, 20016, 20016, 20016,
    ],
    [
      20002, 20002, 20001, 20002, 20029, 20029, 20029, 20016, 20015, 20042,
      20015, 20029, 20002, 20001, 20001, 20002, 20071, 20015, 20016, 20042,
    ],
    [
      20015, 20015, 20015, 20002, 20001, 20002, 20002, 20029, 20029, 20015,
      20014, 20014, 20002, 20029, 20029, 20029, 20015, 20016, 20016, 20042,
    ],
    [
      20028, 20028, 20015, 20002, 20002, 20002, 20001, 20015, 20015, 20015,
      20015, 20001, 20015, 20015, 20015, 20015, 20015, 20015, 20042, 20042,
    ],
    [
      20028, 20028, 20015, 20015, 20014, 20001, 20002, 20014, 20015, 20002,
      20002, 20002, 20015, 20016, 20042, 20015, 20042, 20042, 20042, 20042,
    ],
    [
      20028, 20028, 20028, 20028, 20015, 20002, 20001, 20002, 20002, 20001,
      20001, 20015, 20016, 20016, 20042, 20042, 20042, 20015, 20042, 20016,
    ],
    [
      20028, 20028, 20028, 20028, 20028, 20015, 20015, 20002, 20002, 20015,
      20015, 20015, 20015, 20016, 20015, 20042, 20015, 20015, 20015, 20016,
    ],
    [
      20028, 20028, 20028, 20028, 20028, 20015, 20014, 20002, 20002, 20001,
      20015, 20014, 20015, 20015, 20015, 20015, 20015, 20016, 20016, 20016,
    ],
    [
      20028, 20015, 20015, 20028, 20028, 20016, 20015, 20014, 20002, 20002,
      20001, 20014, 20029, 20015, 20042, 20042, 20042, 20016, 20015, 20015,
    ],
    [
      20015, 20015, 20015, 20028, 20028, 20028, 20016, 20029, 20001, 20002,
      20002, 20014, 20029, 20015, 20042, 20042, 20042, 20016, 20016, 20015,
    ],
    [
      20028, 20015, 20015, 20016, 20028, 20028, 20016, 20015, 20029, 20002,
      20002, 20015, 20015, 20015, 20015, 20042, 20042, 20042, 20016, 20016,
    ],
    [
      20028, 20015, 20016, 20015, 20028, 20028, 20015, 20029, 20029, 20002,
      20002, 20002, 20015, 20016, 20015, 20042, 20042, 20042, 20042, 20016,
    ],
    [
      20015, 20016, 20028, 20028, 20028, 20028, 20015, 20029, 20029, 20001,
      20002, 20002, 20015, 20015, 20015, 20015, 20042, 20042, 20042, 20042,
    ],
    [
      20028, 20028, 20015, 20029, 20001, 20002, 20002, 20001, 20002, 20002,
      20002, 20002, 20002, 20015, 20014, 20016, 20016, 20015, 20016, 20015,
    ],
    [
      20028, 20029, 20029, 20001, 20002, 20002, 20002, 20002, 20002, 20014,
      20015, 20002, 20002, 20014, 20029, 20015, 20015, 20015, 20015, 20015,
    ],
    [
      20015, 20029, 20002, 20002, 20002, 20014, 20014, 20014, 20015, 20015,
      20015, 20015, 20002, 20002, 20001, 20001, 20015, 20015, 20015, 20015,
    ],
    [
      20028, 20029, 20002, 20002, 20014, 20016, 20015, 20016, 20016, 20028,
      20028, 20016, 20002, 20002, 20002, 20001, 20016, 20016, 20015, 20016,
    ],
    [
      20015, 20001, 20002, 20002, 20015, 20015, 20015, 20015, 20028, 20028,
      20028, 20028, 20042, 20042, 20002, 20002, 20015, 20015, 20015, 20015,
    ],
    [
      20029, 20029, 20002, 20001, 20002, 20015, 20016, 20016, 20028, 20028,
      20028, 20028, 20028, 20042, 20002, 20002, 20001, 20015, 20016, 20016,
    ],
    [
      20015, 20029, 20002, 20002, 20002, 20002, 20014, 20016, 20028, 20028,
      20028, 20028, 20028, 20028, 20016, 20002, 20001, 20014, 20016, 20016,
    ],
    [
      20028, 20014, 20002, 20002, 20002, 20002, 20014, 20028, 20028, 20028,
      20028, 20028, 20028, 20028, 20028, 20002, 20002, 20014, 20029, 20016,
    ],
    [
      20028, 20015, 20014, 20015, 20001, 20002, 20016, 20028, 20028, 20028,
      20028, 20028, 20028, 20028, 20028, 20002, 20002, 20029, 20029, 20016,
    ],
    [
      20016, 20016, 20015, 20014, 20002, 20001, 20015, 20028, 20028, 20028,
      20028, 20028, 20028, 20028, 20028, 20002, 20002, 20014, 20016, 20016,
    ],
    [
      20016, 20016, 20015, 20014, 20002, 20002, 20002, 20015, 20015, 20028,
      20028, 20028, 20028, 20028, 20016, 20001, 20002, 20014, 20015, 20015,
    ],
    [
      20016, 20015, 20016, 20015, 20002, 20002, 20002, 20029, 20014, 20028,
      20028, 20028, 20028, 20028, 20015, 20002, 20002, 20016, 20016, 20015,
    ],
    [
      20015, 20015, 20015, 20015, 20015, 20002, 20001, 20014, 20029, 20028,
      20028, 20028, 20028, 20028, 20014, 20002, 20002, 20015, 20015, 20015,
    ],
    [
      20028, 20016, 20015, 20016, 20002, 20001, 20002, 20014, 20029, 20028,
      20028, 20028, 20028, 20028, 20014, 20029, 20001, 20002, 20015, 20016,
    ],
    [
      20028, 20016, 20015, 20002, 20002, 20002, 20002, 20002, 20015, 20015,
      20028, 20028, 20028, 20028, 20028, 20029, 20002, 20002, 20001, 20016,
    ],
    [
      20028, 20015, 20002, 20002, 20002, 20002, 20001, 20002, 20016, 20015,
      20028, 20028, 20028, 20028, 20028, 20029, 20002, 20002, 20001, 20015,
    ],
    [
      20028, 20016, 20002, 20016, 20015, 20002, 20002, 20002, 20015, 20016,
      20016, 20028, 20028, 20028, 20015, 20002, 20002, 20001, 20016, 20015,
    ],
    [
      20015, 20015, 20002, 20015, 20014, 20001, 20002, 20002, 20016, 20015,
      20015, 20016, 20015, 20016, 20016, 20002, 20002, 20002, 20015, 20015,
    ],
    [
      20028, 20015, 20002, 20016, 20015, 20001, 20002, 20002, 20002, 20016,
      20016, 20015, 20016, 20016, 20016, 20001, 20002, 20002, 20015, 20015,
    ],
    [
      20028, 20016, 20002, 20015, 20015, 20002, 20002, 20001, 20002, 20015,
      20015, 20015, 20015, 20016, 20001, 20002, 20002, 20015, 20015, 20015,
    ],
    [
      20028, 20015, 20002, 20002, 20002, 20002, 20002, 20002, 20002, 20014,
      20015, 20016, 20016, 20016, 20002, 20002, 20002, 20015, 20015, 20015,
    ],
    [
      20028, 20028, 20016, 20015, 20002, 20002, 20002, 20002, 20001, 20015,
      20016, 20016, 20015, 20014, 20002, 20001, 20002, 20002, 20001, 20002,
    ],
    [
      20016, 20028, 20015, 20015, 20015, 20002, 20002, 20002, 20001, 20015,
      20015, 20015, 20015, 20015, 20002, 20002, 20002, 20002, 20001, 20002,
    ],
    [
      20016, 20028, 20028, 20028, 20016, 20002, 20002, 20002, 20002, 20002,
      20015, 20015, 20014, 20002, 20001, 20002, 20016, 20015, 20015, 20015,
    ],
    [
      20016, 20015, 20016, 20028, 20016, 20015, 20002, 20002, 20002, 20002,
      20002, 20001, 20001, 20002, 20002, 20002, 20015, 20016, 20016, 20016,
    ],
    [
      20015, 20015, 20015, 20028, 20015, 20015, 20015, 20002, 20001, 20002,
      20002, 20002, 20002, 20002, 20002, 20002, 20015, 20016, 20015, 20015,
    ],
    [
      20015, 20016, 20016, 20028, 20015, 20016, 20016, 20015, 20015, 20002,
      20002, 20002, 20002, 20002, 20002, 20001, 20015, 20016, 20016, 20015,
    ],
    [
      20028, 20028, 20028, 20028, 20016, 20015, 20015, 20015, 20015, 20002,
      20002, 20002, 20002, 20015, 20002, 20001, 20016, 20015, 20015, 20015,
    ],
    [
      20028, 20016, 20016, 20015, 20016, 20016, 20015, 20016, 20016, 20002,
      20002, 20002, 20002, 20015, 20002, 20002, 20015, 20016, 20015, 20016,
    ],
    [
      20028, 20015, 20016, 20016, 20016, 20015, 20016, 20016, 20016, 20015,
      20002, 20002, 20015, 20015, 20002, 20002, 20015, 20015, 20015, 20015,
    ],
    [
      20028, 20028, 20028, 20015, 20015, 20015, 20015, 20015, 20015, 20015,
      20015, 20015, 20016, 20001, 20002, 20002, 20015, 20015, 20016, 20016,
    ],
    [
      20016, 20015, 20028, 20028, 20015, 20016, 20016, 20016, 20015, 20015,
      20016, 20015, 20016, 20002, 20002, 20015, 20016, 20016, 20016, 20016,
    ],
    [
      20016, 20016, 20016, 20028, 20028, 20016, 20016, 20015, 20015, 20015,
      20015, 20014, 20002, 20002, 20015, 20016, 20015, 20016, 20015, 20016,
    ],
    [
      20016, 20016, 20015, 20015, 20028, 20028, 20015, 20016, 20015, 20001,
      20002, 20002, 20002, 20001, 20015, 20015, 20015, 20015, 20016, 20016,
    ],
    [
      20016, 20016, 20015, 20016, 20016, 20028, 20016, 20015, 20016, 20001,
      20002, 20002, 20002, 20016, 20015, 20015, 20016, 20016, 20016, 20016,
    ],
    [
      20016, 20016, 20016, 20016, 20028, 20028, 20015, 20015, 20015, 20002,
      20002, 20015, 20015, 20015, 20015, 20015, 20016, 20002, 341, 20015,
    ],
    [
      20016, 20016, 20016, 20028, 20028, 20016, 20015, 20016, 20001, 20002,
      20002, 20016, 20016, 20015, 20016, 20016, 20015, 20002, 20015, 20016,
    ],
    [
      20016, 20015, 20028, 20028, 20015, 20016, 20016, 20015, 20001, 20002,
      20015, 20015, 20015, 20015, 20015, 20016, 20015, 20002, 20002, 20015,
    ],
    [
      20016, 20028, 20028, 20016, 20016, 20015, 20015, 20015, 20015, 20001,
      20002, 20014, 20015, 20014, 20014, 20015, 20014, 20002, 20002, 20016,
    ],
    [
      20016, 20028, 20016, 20015, 20015, 20015, 20015, 20016, 20016, 20002,
      20002, 20002, 20002, 20002, 20002, 20002, 20002, 20001, 20001, 20016,
    ],
    [
      20016, 20028, 20028, 20028, 20015, 20016, 20016, 20016, 20015, 20014,
      20002, 20002, 20002, 20001, 20002, 20002, 20002, 20001, 20002, 20016,
    ],
    [
      20016, 20028, 20028, 20028, 20016, 20016, 20015, 20015, 20015, 20015,
      20014, 20015, 20014, 20014, 20016, 20015, 20014, 20016, 20015, 20016,
    ],
    [
      20016, 20016, 20016, 20028, 20028, 20015, 20015, 20015, 20016, 20015,
      20015, 20015, 20015, 20016, 20015, 20016, 20015, 20015, 20015, 20015,
    ],
  ],
  bgmap: [],
  fgmap: [],
};
main.floors.MT8 = {
  floorId: "MT8",
  title: "主塔 8 层",
  name: "8",
  width: 36,
  height: 36,
  canFlyTo: true,
  canUseQuickShop: true,
  cannotViewMap: false,
  cannotMoveDirectly: false,
  images: [],
  ratio: 1,
  defaultGround: "X20044",
  firstArrive: [],
  eachArrive: [
    {
      type: "if",
      condition: "(flag:enemyId>10)",
      true: [
        "\t[家丁,role11.png]兄弟们为我报仇！",
        {
          type: "function",
          async: true,
          function:
            'function(){\ncore.battleStart(core.getFlag("enemyId") - 10, 1);\n}',
        },
      ],
    },
    {
      type: "if",
      condition: "(flag:ret1==1)",
      true: [
        { type: "setValue", name: "flag:ret1", value: "2", norefresh: true },
        "\t[hero]老人家，你怎么样了？伤势严重吗？",
        "\t[hero]哎呀，不好，老人已经说不出话来啦，怎么办？对了，我这里有千年雪莲，也许能救活他。",
        "\t[？？,role9.png]…………",
        "\t[？？,role9.png]哎呀，我这是在哪？我还活着吗？",
        "\t[hero]老人家，你终于醒过来了。",
        "\t[司马一,role9.png]少侠，谢谢搭救！以后有需要的时候，我司马一定会全力帮助你的。",
        "\t[hero]原来你就是司马一啊，请帮我看看这封信吧。",
        "\t[司马一,role9.png]这封信是用西域一种失传已久的梵文写成的，收信人是当朝宰相姜慧大人，但很多文字都已经模糊了，你还是去找宰相大人问个清楚吧。",
        "\t[hero]谢谢你！那我赶紧去宰相府了。",
      ],
    },
    {
      type: "if",
      condition: "(flag:ret2==1)",
      true: [
        "\t[宰相,role10.png]快住手！这位少侠，你这把剑是从哪里来得？",
        "\t[hero]这是我父亲留给我的，难道你认识我的父亲？",
        "\t[宰相,role10.png]你父亲宋遇生前是一品大学士，我和他是多年至交。他为人正直，不幸被奸人所害，这么多年来我一直在找你。今天我终于找到你了，可惜我的女儿又被歹人虏走了。",
        "\t[hero]就是您的家丁刚才追的那些人吗？我去找他们，帮您把女儿救回来！",
        "\t[宰相,role10.png]那些人武功高强，心狠手辣，你可要小心了！",
        "\t[hero]（难道这件事情和“仙女姐姐”有关？我一定要查个水落石出！）",
        { type: "setCurtain", color: [0, 0, 0, 1], time: 0, keep: true },
        "慕容天能否顺利地救出宰相的女儿？那个“仙女姐姐”到底是谁？慕容天的父亲又是被谁所害？\n《傲世奇侠传2》将为您揭开谜底，敬请期待！",
        { type: "setValue", name: "status:hp", value: "flag:lv" },
        { type: "win", reason: "恭喜通关", norank: 1 },
      ],
    },
  ],
  parallelDo: "",
  events: {
    "17,4": [
      "\t[hero]咦，刚才走过去的那位女子长得好美啊，像画中的仙女一样！真希望以后还能遇到她！",
      { type: "hide", loc: [[17, 4]] },
      { type: "hide", loc: [[18, 4]] },
    ],
    "18,4": [
      "\t[hero]咦，刚才走过去的那位女子长得好美啊，像画中的仙女一样！真希望以后还能遇到她！",
      { type: "hide", loc: [[17, 4]] },
      { type: "hide", loc: [[18, 4]] },
    ],
    "7,4": [
      {
        type: "if",
        condition: "flag:pot4",
        true: ["已经检查过了啊！！！"],
        false: [
          {
            type: "setValue",
            name: "item:greenPotion",
            value: "(item:greenPotion+1)",
            norefresh: true,
          },
          "得到一个还魂丹。",
          { type: "setValue", name: "flag:pot4", value: "1", norefresh: true },
        ],
      },
    ],
    "29,31": [
      {
        type: "if",
        condition: "flag:pot5",
        true: ["已经检查过了啊！！！"],
        false: [
          {
            type: "setValue",
            name: "item:yellowPotion",
            value: "item:yellowPotion+1",
            norefresh: true,
          },
          "得到一个人参果。",
          { type: "setValue", name: "flag:pot5", value: "1", norefresh: true },
        ],
      },
    ],
    "31,19": ["\t[？？,role8.png]... ...？？我没有话要和你说。"],
    "20,5": ["\t[？？,role7.png]最近京城里频频出事，好多人家都被洗劫一空了！"],
    "4,13": ["\t[？？,role7.png]最近京城里太乱了，我要赶紧回家了！"],
    "13,28": [
      "\t[？？,role6.png]啊，这是什么文字？密密麻麻的，象小蝌蚪一样。我可不认识。",
    ],
    "16,19": [
      "\t[？？,role8.png]你是要抢钱吗？我可没钱了，都被你们抢了4次了。",
    ],
    "17,17": [
      "有家店铺",
      { type: "hide", loc: [[17, 17]] },
      { type: "hide", loc: [[17, 18]] },
      { type: "hide", loc: [[18, 17]] },
      { type: "hide", loc: [[18, 18]] },
    ],
    "18,17": [
      "有家店铺",
      { type: "hide", loc: [[17, 17]] },
      { type: "hide", loc: [[17, 18]] },
      { type: "hide", loc: [[18, 17]] },
      { type: "hide", loc: [[18, 18]] },
    ],
    "17,18": [
      "有家店铺",
      { type: "hide", loc: [[17, 17]] },
      { type: "hide", loc: [[17, 18]] },
      { type: "hide", loc: [[18, 17]] },
      { type: "hide", loc: [[18, 18]] },
    ],
    "18,18": [
      "有家店铺",
      { type: "hide", loc: [[17, 17]] },
      { type: "hide", loc: [[17, 18]] },
      { type: "hide", loc: [[18, 17]] },
      { type: "hide", loc: [[18, 18]] },
    ],
    "28,28": [
      {
        type: "if",
        condition: "((flag:ret1==2)||(flag:ret1==3))",
        true: ["\t[hero]让司马先生好好休息吧，还是不要打扰他了。"],
        false: [
          "\t[hero]你们怎么这么多人欺负一个老人家！给我住手！",
          "\t[家丁,role11.png]哪里来得野小子，不要多管闲事，找打啊！",
          {
            type: "function",
            async: true,
            function: "function(){\ncore.battleStart(28, 1);\n}",
          },
        ],
      },
    ],
    "25,16": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [
        {
          type: "if",
          condition: "(flag:ret1==2)",
          true: [
            {
              type: "setValue",
              name: "flag:ret1",
              value: "3",
              norefresh: true,
            },
            "\t[hero]咦，刚才往南边跑过去的那个人是谁？好像我在城门口看见的“仙女姐姐”呀！她为什么这么匆忙？",
            "\t[家丁,role11.png]抓住她，别让她跑了！追啊！",
            "\t[hero]这些人看起来真凶啊！不行，我得去帮忙！",
            { type: "hide", loc: [[16, 32]], remove: true },
            { type: "hide", loc: [[17, 32]], remove: true },
            { type: "hide", loc: [[18, 32]], remove: true },
            { type: "hide", loc: [[19, 32]], remove: true },
            { type: "setBlock", number: "X20012", loc: [[17, 34]] },
            { type: "setBlock", number: "X20026", loc: [[17, 35]] },
            { type: "setBlock", number: "X20040", loc: [[18, 34]] },
            { type: "setBlock", number: "X20054", loc: [[18, 35]] },
          ],
          false: [
            {
              type: "if",
              condition: "(flag:ret1==3)",
              true: ["\t[hero]我要去救“仙女姐姐”先！！！"],
              false: ["\t[家丁,role11.png]宰相府，闲人莫入！"],
            },
          ],
        },
      ],
    },
    "17,32": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [
        {
          type: "if",
          condition: "(flag:ret1==2)",
          true: ["\t[hero]我要先去宰相府找宰相啊！！！"],
          false: ["\t[hero]我要快点去找“司马一”啊！"],
        },
      ],
    },
    "18,32": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [
        {
          type: "if",
          condition: "(flag:ret1==2)",
          true: ["\t[hero]我要先去宰相府找宰相啊！！！"],
          false: ["\t[hero]我要快点去找“司马一”啊！"],
        },
      ],
    },
    "17,34": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(39, 1);\n}",
      },
    ],
    "18,34": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(39, 1);\n}",
      },
    ],
    "16,34": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(39, 1);\n}",
      },
    ],
    "19,34": [
      {
        type: "function",
        async: true,
        function: "function(){\ncore.battleStart(39, 1);\n}",
      },
    ],
    "13,14": [{ type: "openShop", id: "itemShop", open: true }],
    "14,14": [{ type: "openShop", id: "itemShop", open: true }],
    "16,32": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [
        {
          type: "if",
          condition: "(flag:ret1==2)",
          true: ["\t[hero]我要先去宰相府找宰相啊！！！"],
          false: ["\t[hero]我要快点去找“司马一”啊！"],
        },
      ],
    },
    "19,32": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [
        {
          type: "if",
          condition: "(flag:ret1==2)",
          true: ["\t[hero]我要先去宰相府找宰相啊！！！"],
          false: ["\t[hero]我要快点去找“司马一”啊！"],
        },
      ],
    },
    "17,0": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [
        {
          type: "changeFloor",
          floorId: "MT7",
          loc: [18, 38],
          direction: "left",
        },
      ],
    },
    "18,0": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [
        {
          type: "changeFloor",
          floorId: "MT7",
          loc: [18, 38],
          direction: "left",
        },
      ],
    },
    "26,16": {
      trigger: null,
      enable: true,
      noPass: true,
      displayDamage: true,
      data: [
        {
          type: "if",
          condition: "(flag:ret1==2)",
          true: [
            {
              type: "setValue",
              name: "flag:ret1",
              value: "3",
              norefresh: true,
            },
            "\t[hero]咦，刚才往南边跑过去的那个人是谁？好像我在城门口看见的“仙女姐姐”呀！她为什么这么匆忙？",
            "\t[家丁,role11.png]抓住她，别让她跑了！追啊！",
            "\t[hero]这些人看起来真凶啊！不行，我得去帮忙！",
            { type: "hide", loc: [[16, 32]], remove: true },
            { type: "hide", loc: [[17, 32]], remove: true },
            { type: "hide", loc: [[18, 32]], remove: true },
            { type: "hide", loc: [[19, 32]], remove: true },
            { type: "setBlock", number: "X20012", loc: [[17, 34]] },
            { type: "setBlock", number: "X20026", loc: [[17, 35]] },
            { type: "setBlock", number: "X20040", loc: [[18, 34]] },
            { type: "setBlock", number: "X20054", loc: [[18, 35]] },
          ],
          false: [
            {
              type: "if",
              condition: "(flag:ret1==3)",
              true: ["\t[hero]我要去救“仙女姐姐”先！！！"],
              false: ["\t[家丁,role11.png]宰相府，闲人莫入！"],
            },
          ],
        },
      ],
    },
  },
  changeFloor: {},
  afterBattle: {},
  afterGetItem: {},
  afterOpenDoor: {},
  autoEvent: {},
  cannotMove: {},
  map: [
    [
      20016, 20016, 20028, 20028, 20028, 20028, 20016, 20016, 20016, 20016,
      20028, 20028, 20016, 20016, 20016, 20027, 20044, 20043, 20043, 20044,
      20027, 20042, 20042, 20042, 20042, 20042, 20042, 20042, 20042, 20042,
      20028, 20028, 20028, 20028, 20016, 20016,
    ],
    [
      20027, 20016, 20027, 20016, 20027, 20016, 20027, 20016, 20027, 20016,
      20027, 20028, 20027, 20016, 20027, 20027, 20044, 20043, 20043, 20044,
      20027, 20027, 20042, 20027, 20042, 20027, 20042, 20027, 20016, 20027,
      20016, 20027, 20016, 20027, 20016, 20027,
    ],
    [
      20027, 20027, 20027, 20027, 20027, 20027, 20027, 20027, 20027, 20027,
      20027, 20027, 20027, 20027, 20027, 20027, 20044, 20043, 20043, 20044,
      20027, 20027, 20027, 20027, 20027, 20027, 20027, 20027, 20027, 20027,
      20027, 20027, 20027, 20027, 20027, 20027,
    ],
    [
      20030, 20027, 20015, 20015, 20006, 20007, 20008, 20015, 20006, 20007,
      20008, 20028, 20028, 20015, 20044, 20027, 20044, 20043, 20043, 20044,
      20027, 20015, 20015, 20003, 20004, 20005, 20070, 20003, 20004, 20005,
      20015, 20003, 20004, 20005, 20027, 20030,
    ],
    [
      20015, 20027, 20015, 20016, 20020, 20021, 20022, 20071, 20020, 20021,
      20022, 20015, 20028, 20028, 20044, 20044, 20044, 20043, 20043, 20015,
      20062, 20015, 20015, 20017, 20018, 20019, 20015, 20017, 20018, 20019,
      20015, 20017, 20018, 20019, 20027, 20015,
    ],
    [
      20015, 20027, 20015, 20044, 20044, 20044, 20044, 20044, 20044, 20044,
      20015, 20044, 20028, 20028, 20015, 20015, 20015, 20043, 20043, 20015,
      20076, 20044, 20044, 20044, 20044, 20044, 20044, 20044, 20044, 20044,
      20044, 20044, 20044, 20044, 20027, 20015,
    ],
    [
      20029, 20027, 20043, 20043, 20043, 20043, 20043, 20043, 20043, 20043,
      20043, 20043, 20032, 20043, 20043, 20043, 20043, 20043, 20043, 20043,
      20043, 20043, 20043, 20043, 20043, 20043, 20043, 20043, 20043, 20043,
      20043, 20043, 20043, 20043, 20027, 20029,
    ],
    [
      20014, 20027, 20043, 20043, 20043, 20043, 20043, 20043, 20043, 20043,
      20043, 20043, 20046, 20043, 20043, 20043, 20043, 20043, 20043, 20043,
      20043, 20043, 20043, 20043, 20006, 20007, 20007, 20008, 20043, 20043,
      20043, 20043, 20043, 20043, 20027, 20014,
    ],
    [
      20014, 20027, 20043, 20043, 20044, 20044, 20038, 20007, 20007, 20044,
      20015, 20028, 20028, 20015, 20003, 20004, 20005, 20043, 20043, 20044,
      20044, 20044, 20038, 20006, 20007, 20007, 20007, 20007, 20008, 20038,
      20044, 20044, 20043, 20043, 20027, 20014,
    ],
    [
      20014, 20027, 20043, 20043, 20015, 20052, 20048, 20017, 20019, 20011,
      20072, 20028, 20028, 20044, 20017, 20018, 20019, 20043, 20043, 20044,
      20015, 20052, 20034, 20020, 20018, 20021, 20021, 20018, 20022, 20034,
      20011, 20015, 20043, 20043, 20027, 20014,
    ],
    [
      20014, 20027, 20043, 20043, 20044, 20006, 20007, 20008, 20007, 20007,
      20015, 20028, 20028, 20015, 20044, 20044, 20044, 20043, 20043, 20044,
      20016, 20052, 20030, 20016, 20044, 20044, 20044, 20044, 20016, 20030,
      20011, 20016, 20043, 20043, 20027, 20014,
    ],
    [
      20014, 20027, 20043, 20043, 20044, 20020, 20018, 20022, 20018, 20021,
      20028, 20028, 20028, 20015, 20007, 20007, 20044, 20043, 20043, 20044,
      20015, 20052, 20029, 20016, 20044, 20044, 20044, 20044, 20016, 20029,
      20011, 20015, 20043, 20043, 20027, 20014,
    ],
    [
      20014, 20027, 20043, 20043, 20062, 20044, 20044, 20044, 20044, 20015,
      20028, 20015, 20006, 20007, 20007, 20008, 20011, 20043, 20043, 20044,
      20016, 20052, 20030, 20007, 20007, 20007, 20007, 20007, 20007, 20030,
      20011, 20016, 20043, 20043, 20027, 20014,
    ],
    [
      20014, 20027, 20043, 20043, 20076, 20030, 20044, 20003, 20004, 20005,
      20028, 20015, 20033, 20009, 20010, 20035, 20008, 20043, 20043, 20044,
      20015, 20006, 20007, 20007, 20007, 20007, 20007, 20007, 20007, 20007,
      20008, 20015, 20043, 20043, 20027, 20014,
    ],
    [
      20029, 20027, 20043, 20043, 20030, 20044, 20044, 20017, 20018, 20019,
      20028, 20016, 20047, 20023, 20023, 20060, 20049, 20043, 20043, 20044,
      20016, 20033, 20056, 20056, 20036, 20057, 20058, 20037, 20056, 20056,
      20035, 20016, 20043, 20043, 20027, 20029,
    ],
    [
      20030, 20027, 20043, 20043, 20044, 20044, 20015, 20044, 20044, 20044,
      20028, 20015, 20016, 20044, 20044, 20044, 20044, 20043, 20043, 20016,
      20042, 20047, 20048, 20048, 20050, 20023, 20024, 20051, 20048, 20048,
      20049, 20015, 20043, 20043, 20027, 20030,
    ],
    [
      20030, 20027, 20043, 20043, 20044, 20044, 20044, 20044, 20044, 20015,
      20028, 20028, 20028, 20015, 20044, 20044, 20044, 20043, 20043, 20044,
      20044, 20044, 20044, 20015, 20015, 20043, 20043, 20015, 20015, 20044,
      20044, 20044, 20043, 20043, 20027, 20030,
    ],
    [
      20030, 20027, 20043, 20043, 20043, 20043, 20043, 20043, 20043, 20043,
      20043, 20043, 20032, 20043, 20043, 20043, 20043, 20043, 20043, 20043,
      20043, 20043, 20043, 20043, 20043, 20043, 20043, 20043, 20043, 20043,
      20043, 20043, 20043, 20043, 20027, 20030,
    ],
    [
      20030, 20027, 20043, 20043, 20043, 20043, 20043, 20043, 20043, 20043,
      20043, 20043, 20046, 20043, 20043, 20043, 20063, 20043, 20043, 20043,
      20043, 20043, 20043, 20043, 20043, 20043, 20043, 20043, 20043, 20043,
      20043, 20063, 20043, 20043, 20027, 20030,
    ],
    [
      20029, 20027, 20043, 20043, 20044, 20044, 20044, 20044, 20044, 20044,
      20044, 20015, 20028, 20044, 20044, 20044, 20077, 20043, 20043, 20044,
      20006, 20007, 20008, 20015, 20044, 20044, 20044, 20044, 20044, 20044,
      20044, 20077, 20043, 20043, 20027, 20029,
    ],
    [
      20014, 20027, 20043, 20043, 20044, 20015, 20038, 20007, 20007, 20038,
      20015, 20028, 20028, 20015, 20003, 20004, 20005, 20043, 20043, 20044,
      20020, 20018, 20022, 20044, 20044, 20038, 20006, 20007, 20007, 20008,
      20038, 20044, 20043, 20043, 20027, 20014,
    ],
    [
      20014, 20027, 20043, 20043, 20015, 20052, 20034, 20020, 20022, 20034,
      20011, 20028, 20028, 20044, 20017, 20018, 20019, 20043, 20043, 20044,
      20044, 20044, 20044, 20015, 20052, 20034, 20020, 20018, 20018, 20022,
      20034, 20011, 20043, 20043, 20027, 20014,
    ],
    [
      20014, 20027, 20043, 20043, 20044, 20006, 20007, 20007, 20007, 20007,
      20008, 20028, 20028, 20015, 20044, 20044, 20044, 20043, 20043, 20006,
      20007, 20007, 20008, 20044, 20052, 20030, 20002, 20044, 20044, 20002,
      20030, 20011, 20043, 20043, 20027, 20014,
    ],
    [
      20014, 20027, 20043, 20043, 20015, 20020, 20018, 20021, 20021, 20018,
      20022, 20028, 20028, 20028, 20028, 20015, 20044, 20043, 20043, 20020,
      20018, 20018, 20022, 20015, 20052, 20030, 20006, 20007, 20007, 20008,
      20030, 20011, 20043, 20043, 20027, 20014,
    ],
    [
      20014, 20027, 20043, 20043, 20044, 20044, 20044, 20044, 20044, 20016,
      20016, 20044, 20015, 20028, 20028, 20044, 20044, 20043, 20043, 20044,
      20044, 20044, 20072, 20072, 20006, 20007, 20007, 20007, 20007, 20007,
      20007, 20008, 20043, 20043, 20027, 20014,
    ],
    [
      20014, 20027, 20043, 20043, 20006, 20007, 20008, 20044, 20006, 20007,
      20008, 20016, 20044, 20028, 20028, 20015, 20044, 20043, 20043, 20030,
      20006, 20007, 20008, 20015, 20033, 20056, 20036, 20009, 20010, 20037,
      20056, 20035, 20043, 20043, 20027, 20014,
    ],
    [
      20014, 20027, 20043, 20043, 20020, 20018, 20022, 20015, 20020, 20018,
      20022, 20044, 20015, 20028, 20028, 20044, 20030, 20043, 20043, 20044,
      20020, 20018, 20022, 20044, 20047, 20048, 20050, 20023, 20024, 20051,
      20048, 20049, 20043, 20043, 20027, 20014,
    ],
    [
      20014, 20027, 20043, 20043, 20044, 20044, 20044, 20044, 20044, 20044,
      20015, 20028, 20028, 20061, 20028, 20015, 20044, 20043, 20043, 20044,
      20044, 20030, 20044, 20030, 20044, 20044, 20044, 20044, 20068, 20044,
      20044, 20044, 20043, 20043, 20027, 20014,
    ],
    [
      20014, 20027, 20043, 20043, 20043, 20043, 20043, 20043, 20043, 20043,
      20043, 20032, 20043, 20075, 20043, 20043, 20043, 20043, 20043, 20043,
      20043, 20043, 20043, 20043, 20043, 20043, 20043, 20043, 20082, 20043,
      20043, 20043, 20043, 20043, 20027, 20014,
    ],
    [
      20014, 20027, 20043, 20043, 20038, 20007, 20007, 20043, 20043, 20043,
      20043, 20046, 20043, 20043, 20043, 20043, 20043, 20043, 20043, 20043,
      20043, 20043, 20043, 20043, 20043, 20043, 20043, 20043, 20043, 20043,
      20043, 20043, 20043, 20043, 20027, 20014,
    ],
    [
      20029, 20027, 20044, 20006, 20007, 20007, 20008, 20011, 20044, 20015,
      20028, 20028, 20003, 20004, 20005, 20015, 20044, 20043, 20043, 20044,
      20044, 20044, 20044, 20044, 20044, 20044, 20044, 20044, 20044, 20044,
      20044, 20044, 20043, 20043, 20027, 20029,
    ],
    [
      20015, 20027, 20015, 20033, 20009, 20010, 20035, 20008, 20015, 20028,
      20028, 20015, 20017, 20018, 20019, 20002, 20044, 20043, 20043, 20044,
      20044, 20044, 20003, 20004, 20005, 20015, 20003, 20004, 20005, 20071,
      20003, 20004, 20005, 20015, 20027, 20015,
    ],
    [
      20015, 20027, 20015, 20047, 20023, 20024, 20049, 20049, 20028, 20028,
      20015, 20002, 20002, 20002, 20015, 20027, 20044, 20043, 20043, 20044,
      20027, 20015, 20017, 20018, 20019, 20070, 20017, 20018, 20019, 20015,
      20017, 20018, 20019, 20015, 20027, 20015,
    ],
    [
      20027, 20027, 20027, 20027, 20027, 20027, 20027, 20027, 20027, 20027,
      20027, 20027, 20027, 20027, 20027, 20027, 20044, 20043, 20043, 20044,
      20027, 20027, 20027, 20027, 20027, 20027, 20027, 20027, 20027, 20027,
      20027, 20027, 20027, 20027, 20027, 20027,
    ],
    [
      20027, 20016, 20027, 20016, 20027, 20016, 20027, 20028, 20027, 20028,
      20027, 20028, 20027, 20016, 20027, 20027, 20044, 20043, 20043, 20044,
      20027, 20027, 20028, 20027, 20016, 20027, 20028, 20027, 20028, 20027,
      20028, 20027, 20028, 20027, 20016, 20027,
    ],
    [
      20016, 20016, 20016, 20028, 20028, 20028, 20028, 20028, 20028, 20028,
      20028, 20028, 20028, 20028, 20028, 20027, 20044, 20043, 20043, 20044,
      20027, 20028, 20028, 20028, 20028, 20028, 20028, 20028, 20028, 20028,
      20028, 20028, 20028, 20028, 20028, 20028,
    ],
  ],
  bgmap: [],
  fgmap: [],
};

(function () {
  let oldImpl = core.getBuff;
  core.getBuff = function(a) {
    return a === 'atk' ? 1000 : oldImpl(a);
  };
})();