# UtilityAI
UtilityAI plugin for Unreal Engine

# Instructions:
- Add the DecisionMakerComponent to an AI controller.
- Set BT_OptionTree to a BehaviorTree containing BTT_RunOptionBehaviorTree.
- Add an AIOptionSetDataAsset to BaseOptionSets.
- (Optional) override `GetOptionSets` to pull from other sources. For example, equipped weapon might contain options, or you might want to limit which options get evaluated.
- Write some AIConsiderations. I only included one as an example.
