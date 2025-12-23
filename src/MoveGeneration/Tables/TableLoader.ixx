export module Chess.MoveGeneration:TableLoader;

export import :BMI;

export namespace chess {
	MagicMaps<StaticBMI> loadMaps();
}