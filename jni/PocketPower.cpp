#include <jni.h>
#include <dlfcn.h>
#include <android/log.h>
#include <stdlib.h>
#include <substrate.h>

#include "virtualhook.h"

#include "mcpe/client/renderer/tileentity/TileEntityRenderDispatcher.h"
#include "mcpe/client/renderer/tile/TileTessellator.h"
#include "mcpe/client/game/MinecraftClient.h"
#include "mcpe/client/gui/screens/CreativeInventoryScreen.h"
#include "mcpe/client/gui/screens/touch/StartMenuScreen.h"
#include "mcpe/world/level/TilePos.h"
#include "mcpe/world/item/crafting/Recipes.h"
#include "mcpe/world/level/TileSource.h"
#include "mcpe/world/level/tile/TntTile.h"
#include "mcpe/world/level/tile/EntityTile.h"
#include "mcpe/world/level/tile/entity/TileEntity.h"
#include "mcpe/world/item/Item.h"
#include "mcpe/world/item/ItemInstance.h"
#include "mcpe/world/item/TileItem.h"
#include "mcpe/world/item/TilePlanterItem.h"
#include "mcpe/world/material/Material.h"
#include "mcpe/world/entity/player/Player.h"
#include "mcpe/world/Facing.h"

#include "mcperedstone/client/renderer/tileentity/PistonRenderer.h"
#include "mcperedstone/world/level/tile/RedstoneWireTile.h"
#include "mcperedstone/world/level/tile/NotGateTile.h"
#include "mcperedstone/world/level/tile/RepeaterTile.h"
#include "mcperedstone/world/level/tile/LightPressurePlateTile.h"
#include "mcperedstone/world/level/tile/HeavyPressurePlateTile.h"
#include "mcperedstone/world/level/tile/LeverTile.h"
#include "mcperedstone/world/level/tile/LampTile.h"
#include "mcperedstone/world/level/tile/ButtonTile.h"
#include "mcperedstone/world/level/tile/PistonBaseTile.h"
#include "mcperedstone/world/level/tile/PistonArmTile.h"
#include "mcperedstone/world/level/tile/PistonMovingTile.h"
#include "mcperedstone/world/level/tile/RedstoneBlockTile.h"
#include "mcperedstone/world/level/tile/DoorTile.h"
#include "mcperedstone/world/level/tile/TrapDoorTile.h"
#include "mcperedstone/world/level/tile/FenceGateTile.h"
#include "mcperedstone/utils/CreativeTab.h"
#include "mcperedstone/utils/PocketPowerCraftingManager.h"


extern void (*_TileEntity$initTileEntities)();
void TileEntity$initTileEntities();

extern std::unique_ptr<TileEntity> (*_EntityTile$newTileEntity)(EntityTile*, const TilePos&);
std::unique_ptr<TileEntity> EntityTile$newTileEntity(EntityTile*, const TilePos&);

extern std::unique_ptr<TileEntity> (*_TileEntityFactory$createTileEntity)(TileEntityType, const TilePos&);
std::unique_ptr<TileEntity> TileEntityFactory$createTileEntity(TileEntityType, const TilePos&);


bool (*_TileTessellator$tessellateInWorld)(TileTessellator*, Tile*, const TilePos&, unsigned char, bool);
bool TileTessellator$tessellateInWorld(TileTessellator* self, Tile* tile, const TilePos& pos, unsigned char data, bool b) {
	switch(tile->renderType) {
	case 5:
		return self->tessellateRedstoneWireInWorld((RedstoneWireTile*) tile, pos);
	case 12:
		return self->tessellateLeverInWorld((LeverTile*) tile, pos);
	case 15:
		return self->tessellateRepeaterInWorld((RepeaterTile*) tile, pos);
	case 17:
		return self->tessellatePistonArmInWorld((PistonArmTile*) tile, pos, true);
	case 100:
		return self->tessellateLitNotGateInWorld((NotGateTile*) tile, pos);
	}
	
	if(tile->id == 33 || tile->id == 29)
		return self->tessellatePistonBaseInWorld((PistonBaseTile*) tile, pos, false);

	return _TileTessellator$tessellateInWorld(self, tile, pos, data, b);
}

void initTileItems() {
	new TileItem(55 - 0x100);
	new TileItem(75 - 0x100);
	new TileItem(76 - 0x100);
	new TileItem(70 - 0x100);
	new TileItem(72 - 0x100);
	new TileItem(69 - 0x100);
	new TileItem(123 - 0x100);
	new TileItem(124 - 0x100);
	new TileItem(77 - 0x100);
	new TileItem(143 - 0x100);
	new TileItem(147 - 0x100);
	new TileItem(148 - 0x100);
	new TileItem(93 - 0x100);
	new TileItem(94 - 0x100);
	new TileItem(33 - 0x100);
	new TileItem(29 - 0x100);
	new TileItem(34 - 0x100);
	new TileItem(36 - 0x100);
}

void initMaterials() {
	Material::circuits.blocksMotion = false;
	Material::circuits.isSolid = false;
}

void (*_Recipes$init)(Recipes*);
void Recipes$init(Recipes* self) {
	_Recipes$init(self);
	
	PocketPowerCraftingManager::initRecipes(self);
}

void (*_Tile$initTiles)();
void Tile$initTiles() {
	_Tile$initTiles();

	initMaterials();

	Tile::redStoneDust = new RedstoneWireTile(55, "redstone_dust_cross", &Material::circuits);
	Tile::notGate_off = new NotGateTile(75, "redstone_torch_off");
	Tile::notGate_on = new NotGateTile(76, "redstone_torch_on");
	Tile::pressurePlate_stone = new LightPressurePlateTile(70, "pressurePlateStone", "stone", &Material::stone, LightPressurePlateTile::SENSITIVITY::MOBS);
	Tile::pressurePlate_wood = new LightPressurePlateTile(72, "pressurePlateWood", "planks", &Material::wood, LightPressurePlateTile::SENSITIVITY::EVERYTHING);
	Tile::lever = new LeverTile(69, "lever", &Material::circuits);
	Tile::offlamp = new LampTile(123, "redstone_lamp_off", &Material::dirt);
	Tile::onlamp = new LampTile(124, "redstone_lamp_on", &Material::dirt);
	Tile::button = new ButtonTile(77, "stone", &Material::circuits);
	Tile::buttonWood = new ButtonTile(143, "planks", &Material::circuits);
	Tile::pressurePlate_gold = new HeavyPressurePlateTile(147, "weightedPlate_light", "gold_block", &Material::metal, 15);
	Tile::pressurePlate_iron = new HeavyPressurePlateTile(148, "weightedPlate_heavy", "iron_block", &Material::metal, 150);
	Tile::diode_off = new RepeaterTile(93, "repeater_off", false);
	Tile::diode_on = new RepeaterTile(94, "repeater_on", true);
	Tile::pistonNormal = new PistonBaseTile(33, false);
	Tile::pistonSticky = new PistonBaseTile(29, true);
	Tile::pistonArm = new PistonArmTile(34);
	Tile::pistonMoving = new PistonMovingTile(36);
	Tile::redstoneBlock = new RedstoneBlockTile(152);

	initTileItems();
}

void (*_Item$initItems)();
void Item$initItems() {
	Item::repeater = (Item*)(new TilePlanterItem(100, Tile::diode_off));
	Item::repeater->setNameID("diode");
	Item::repeater->setCategory(CreativeTab::ITEMS);
	Item::repeater->setIcon("repeater", 0));

	_Item$initItems();
}

void (*_Item$initCreativeItems)();
void Item$initCreativeItems() {
	CreativeTabWorker::reorderCreativeItems();
	_Item$initCreativeItems();
	Item::addCreativeItem(Item::door_iron, 0);
	Item::addCreativeItem(Item::repeater, 0);
	Item::addCreativeItem(Item::items[76], 0);
	Item::addCreativeItem(Item::items[70], 0);
	Item::addCreativeItem(Item::items[72], 0);
	Item::addCreativeItem(Item::items[147], 0);
	Item::addCreativeItem(Item::items[148], 0);
	Item::addCreativeItem(Item::items[69], 0);
	Item::addCreativeItem(Item::items[77], 0);
	Item::addCreativeItem(Item::items[143], 0);
	Item::addCreativeItem(Item::items[123], 0);
	Item::addCreativeItem(Item::items[33], 0);
	Item::addCreativeItem(Item::items[29], 0);
}

ItemInstance (*_CreativeInventoryScreen$getItemFromType)(CreativeInventoryScreen*, CreativeTab);
ItemInstance CreativeInventoryScreen$getItemFromType(CreativeInventoryScreen* self, CreativeTab type) {
	if(type == CreativeTab::ITEMS) return ItemInstance(331, 1, 0);
	return _CreativeInventoryScreen$getItemFromType(self, type);
}

void (*_Touch$StartMenuScreen$chooseRandomSplash)(Touch::StartMenuScreen*);
void Touch$StartMenuScreen$chooseRandomSplash(Touch::StartMenuScreen* self) {
	self->mSplashes = self->pocketpowerSplashes;
	
	_Touch$StartMenuScreen$chooseRandomSplash(self);
}

bool (*_Item$useOn)(Item*, ItemInstance*, Player*, int, int, int, signed char, float, float, float);
bool Item$useOn(Item* self, ItemInstance* item, Player* player, int x, int y, int z, signed char side, float xx, float yy, float zz) {
	if(item->item == Item::redStone) {
		if(Tile::redStoneDust->mayPlace(&player->region, x + Facing::STEP_X[side], y + Facing::STEP_Y[side], z + Facing::STEP_Z[side])) {
			item->count--;
			player->region.setTileAndData(x + Facing::STEP_X[side], y + Facing::STEP_Y[side], z + Facing::STEP_Z[side], {55, 0}, 3);
			return true;
		}
	}
	return _Item$useOn(self, item, player, x, y, z, side, xx, yy, zz);
}

void* (*_TileEntityRenderDispatcher$C)(TileEntityRenderDispatcher*);
void* TileEntityRenderDispatcher$C(TileEntityRenderDispatcher* self) {
	self->pistonRenderer = new PistonRenderer(self);
	
	return _TileEntityRenderDispatcher$C(self);
}

TileEntityRenderer* (*_TileEntityRenderDispatcher$getRenderer)(TileEntityRenderDispatcher*, TileEntity&, const Vec3&, float, const mce::MaterialPtr&, const std::string&, int);
TileEntityRenderer* TileEntityRenderDispatcher$getRenderer(TileEntityRenderDispatcher* self, TileEntity& tileEntity, const Vec3& pos, float partialTicks, const mce::MaterialPtr& material, const std::string& str, int i) {
	if(tileEntity.rendererId == 100)
		self->pistonRenderer->render(self->minecraft.getLocalPlayer()->region, tileEntity, pos, partialTicks, material, str, i);
		
	return _TileEntityRenderDispatcher$getRenderer(self, tileEntity, pos, partialTicks, material, str, i);
}


JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	
	MSHookFunction((void*) &Tile::initTiles, (void*) &Tile$initTiles, (void**) &_Tile$initTiles);
	MSHookFunction((void*) &Item::initItems, (void*) &Item$initItems, (void**) &_Item$initItems);
	MSHookFunction((void*) &TileTessellator::tessellateInWorld, (void*) &TileTessellator$tessellateInWorld, (void**) &_TileTessellator$tessellateInWorld);
	MSHookFunction((void*) &Item::initCreativeItems, (void*) &Item$initCreativeItems, (void**) &_Item$initCreativeItems);
	MSHookFunction((void*) &Item::useOn, (void*) &Item$useOn, (void**) &_Item$useOn);
	MSHookFunction((void*) &CreativeInventoryScreen::getItemFromType, (void*) &CreativeInventoryScreen$getItemFromType, (void**) &_CreativeInventoryScreen$getItemFromType);
	MSHookFunction((void*) &Touch::StartMenuScreen::chooseRandomSplash, (void*) &Touch$StartMenuScreen$chooseRandomSplash, (void**) &_Touch$StartMenuScreen$chooseRandomSplash);
	MSHookFunction((void*) &Recipes::init, (void*) &Recipes$init, (void**) &_Recipes$init);
	MSHookFunction((void*) &TileEntity::initTileEntities, (void*) &TileEntity$initTileEntities, (void**) &_TileEntity$initTileEntities);
	MSHookFunction((void*) &TileEntityFactory::createTileEntity, (void*) &TileEntityFactory$createTileEntity, (void**) &_TileEntityFactory$createTileEntity);
	MSHookFunction((void*) &EntityTile::newTileEntity, (void*) &EntityTile$newTileEntity, (void**) &_EntityTile$newTileEntity);

	void* TileEntityRenderDispatcher$$getRenderer = (void*) dlsym(RTLD_DEFAULT, "_ZN26TileEntityRenderDispatcher6renderER10TileEntityRK4Vec3fRKN3mce11MaterialPtrERKSsi");
	MSHookFunction(TileEntityRenderDispatcher$$getRenderer, (void*) &TileEntityRenderDispatcher$getRenderer, (void**) &_TileEntityRenderDispatcher$getRenderer);
	void* TileEntityRenderDispatcher$$C = (void*) dlsym(RTLD_DEFAULT, "_ZN26TileEntityRenderDispatcherC2Ev");
	MSHookFunction(TileEntityRenderDispatcher$$C, (void*) &TileEntityRenderDispatcher$C, (void**) &_TileEntityRenderDispatcher$C);

	DoorTile::_$neighborChanged = (void (*)(DoorTile*, TileSource*, int, int, int, int, int, int)) VirtualHook("_ZTV8DoorTile", "_ZN8DoorTile15neighborChangedEP10TileSourceiiiiii", (void*) &DoorTile::$neighborChanged);
	TrapDoorTile::_$neighborChanged = (void (*)(TrapDoorTile*, TileSource*, int, int, int, int, int, int)) VirtualHook("_ZTV12TrapDoorTile", "_ZN12TrapDoorTile15neighborChangedEP10TileSourceiiiiii", (void*) &TrapDoorTile::$neighborChanged);
	VirtualHook("_ZTV7TntTile", "_ZN7TntTile15neighborChangedEP10TileSourceiiiiii", (void*) &TntTile::_neighborChanged);
	VirtualHook("_ZTV7TntTile", "_ZN4Tile7onPlaceEP10TileSourceiii", (void*) &TntTile::onPlace);
	VirtualHook("_ZTV13FenceGateTile", "_ZN13FenceGateTile15neighborChangedEP10TileSourceiiiiii", (void*) &FenceGateTile::_neighborChanged);
	
	return JNI_VERSION_1_2;
}
