#include "Game/Inventory/Inventory.h"
#include "Game/Inventory/ItemCatalog.h"
#include "TestSupport.h"

#include <vector>

namespace {

void itemStacksClampAndSort() {
    Inventory inv;
    inv.addItem("recovery_candy", 2);
    inv.addItem("pixel_screw", 5);
    inv.addItem("recovery_candy", 3);
    inv.addItem("", 9);
    inv.addItem("old_button", -2);

    TestSupport::require(inv.getItemCount("recovery_candy") == 5, "recovery candy stacks");
    TestSupport::require(inv.getItemCount("pixel_screw") == 5, "pixel screw stacks");
    TestSupport::require(inv.getItemCount("old_button") == 0, "negative item count ignored");

    std::vector<ItemStack> stacks = inv.getItemStacks();
    TestSupport::require(stacks.size() == 2, "only positive stacks are saved");
    TestSupport::require(stacks[0].itemId == "pixel_screw", "item stacks sort by id");
    TestSupport::require(stacks[0].count == 5, "first sorted stack keeps count");
    TestSupport::require(stacks[1].itemId == "recovery_candy", "item stacks sort by id second");
    TestSupport::require(stacks[1].count == 5, "second sorted stack keeps count");
}

void consumeItemsRespectsCounts() {
    Inventory inv;
    inv.addItem("trial_token", 3);

    TestSupport::require(inv.consumeItem("trial_token", 2), "consume available items succeeds");
    TestSupport::require(inv.getItemCount("trial_token") == 1, "consume subtracts count");
    TestSupport::require(!inv.consumeItem("trial_token", 2), "consume too many fails");
    TestSupport::require(inv.getItemCount("trial_token") == 1, "failed consume preserves count");
}

void loadItemStacksRestoresOnlyValidEntries() {
    Inventory inv;
    inv.addItem("stale_item", 7);
    inv.loadItemStacks({
        {"color_battery", 2},
        {"", 5},
        {"pixel_screw", 0},
        {"old_button", 4}
    });

    TestSupport::require(inv.getItemCount("color_battery") == 2, "color battery loads");
    TestSupport::require(inv.getItemCount("old_button") == 4, "old button loads");
    TestSupport::require(inv.getItemCount("pixel_screw") == 0, "zero count does not load");
    TestSupport::require(inv.getItemCount("stale_item") == 0, "load clears previous item stacks");
}

void itemCatalogKnowsFirstChapterItems() {
    const std::vector<ItemDef>& allItems = ItemCatalog::all();
    const ItemDef* candy = ItemCatalog::find("recovery_candy");
    const ItemDef* battery = ItemCatalog::find("color_battery");
    const ItemDef* token = ItemCatalog::find("trial_token");
    const ItemDef* screw = ItemCatalog::find("pixel_screw");
    const ItemDef* button = ItemCatalog::find("old_button");
    const ItemDef* relic = ItemCatalog::find("pixel_controller");
    const ItemDef* sticker = ItemCatalog::find("no_pay_victory_sticker");
    const ItemDef* melody = ItemCatalog::find("half_melody_arcade");

    TestSupport::require(allItems.size() == 8, "catalog contains first chapter items");
    TestSupport::require(candy != nullptr, "recovery candy definition exists");
    TestSupport::require(candy->category == ItemCategory::Consumable, "recovery candy category");
    TestSupport::require(candy->childlikeHeartDelta == 30.0f, "recovery candy restores heart");
    TestSupport::require(battery != nullptr && battery->childlikeHeartDelta == 60.0f,
        "color battery restores heart");
    TestSupport::require(battery->grievanceDelta == -10.0f, "color battery lowers grievance");
    TestSupport::require(token != nullptr && token->category == ItemCategory::Story, "trial token is story item");
    TestSupport::require(screw != nullptr && screw->category == ItemCategory::Material, "pixel screw is material");
    TestSupport::require(button != nullptr && button->category == ItemCategory::Material, "old button is material");
    TestSupport::require(relic != nullptr && relic->category == ItemCategory::Relic, "pixel controller is relic");
    TestSupport::require(sticker != nullptr && sticker->category == ItemCategory::HiddenCollectible,
        "no pay sticker is hidden collectible");
    TestSupport::require(melody != nullptr && melody->category == ItemCategory::HiddenCollectible,
        "half melody is hidden collectible");
    TestSupport::require(ItemCatalog::find("missing_item") == nullptr, "unknown item is absent");
}

}  // namespace

int main() {
    itemStacksClampAndSort();
    consumeItemsRespectsCounts();
    loadItemStacksRestoresOnlyValidEntries();
    itemCatalogKnowsFirstChapterItems();
    return 0;
}
