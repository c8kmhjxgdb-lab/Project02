#include "Game/Inventory/ItemCatalog.h"

#include <algorithm>

namespace {

const std::vector<ItemDef>& definitions() {
    static const std::vector<ItemDef> defs = {
        {"recovery_candy", "恢复糖", ItemCategory::Consumable,
            "小卖部味道的星星糖，恢复 30 点童心。", true, true, 0.0f, 30.0f, 0.0f},
        {"color_battery", "彩色电池", ItemCategory::Consumable,
            "让灰掉的世界重新亮一点，恢复 60 点童心并降低 10 点委屈。", true, true, 0.0f, 60.0f, -10.0f},
        {"trial_token", "试玩币", ItemCategory::Story,
            "弹窗游乐厅的试玩币，集齐三枚可开启 Boss 门。", false, false, 0.0f, 0.0f, 0.0f},
        {"pixel_screw", "像素螺丝", ItemCategory::Material,
            "从废铁工厂里找到的发光螺丝。", false, true, 0.0f, 0.0f, 0.0f},
        {"old_button", "旧按键", ItemCategory::Material,
            "旧游戏机上掉下来的按键。", false, true, 0.0f, 0.0f, 0.0f},
        {"pixel_controller", "像素手柄", ItemCategory::Relic,
            "第一章信物，安放到秘密基地后解锁试玩币训练机关。", false, false, 0.0f, 0.0f, 0.0f},
        {"no_pay_victory_sticker", "未充值也能赢贴纸", ItemCategory::HiddenCollectible,
            "没有被诱导按钮带跑的证明。", false, false, 0.0f, 0.0f, 0.0f},
        {"half_melody_arcade", "半截旋律：游乐厅", ItemCategory::HiddenCollectible,
            "隐藏章回声音乐厅的旋律碎片之一。", false, false, 0.0f, 0.0f, 0.0f}
    };
    return defs;
}

}  // namespace

namespace ItemCatalog {

const ItemDef* find(const std::string& itemId) {
    const auto& defs = definitions();
    auto it = std::find_if(defs.begin(), defs.end(),
        [&itemId](const ItemDef& def) { return def.id == itemId; });
    return it == defs.end() ? nullptr : &*it;
}

const std::vector<ItemDef>& all() {
    return definitions();
}

}  // namespace ItemCatalog
