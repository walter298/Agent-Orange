export module Chess.LegalMoveGeneration:TableLoader;

export import :BMI;

export namespace chess {
	MagicMaps<StaticBMI> loadMaps();
}