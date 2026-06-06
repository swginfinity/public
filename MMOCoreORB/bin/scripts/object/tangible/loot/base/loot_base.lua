-- Infinity: base loot template — lazy-loaded when diner_ingredient2 and similar
-- quest food items need their derivation chain resolved

object_tangible_loot_base_shared_loot_base = SharedTangibleObjectTemplate:new {
	clientTemplateFileName = "object/tangible/loot/base/shared_loot_base.iff"
}
ObjectTemplates:addClientTemplate(object_tangible_loot_base_shared_loot_base, "object/tangible/loot/base/shared_loot_base.iff")

object_tangible_loot_base_loot_base = object_tangible_loot_base_shared_loot_base:new {
}
ObjectTemplates:addTemplate(object_tangible_loot_base_loot_base, "object/tangible/loot/base/loot_base.iff")
