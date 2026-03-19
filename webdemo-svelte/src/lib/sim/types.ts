export type SimulationPhase = 'loading' | 'idle' | 'running' | 'error';

export type SimulationStatus = {
  phase: SimulationPhase;
  message: string;
};

export const SPREAD_HISTORY_WINDOW = 500;

export type SpreadSnapshot = {
  bidsMissing: boolean;
  asksMissing: boolean;
  highestBid: number;
  lowestAsk: number;
};

export type DepthBinSnapshot = {
  price: number;
  totalQty: number;
};

export type DepthSnapshot = {
  bidBins: DepthBinSnapshot[];
  askBins: DepthBinSnapshot[];
};

export type ObservationSnapshot = {
  time: number;
  spread: SpreadSnapshot | null;
  depth: DepthSnapshot | null;
};

export type SpreadHistoryPoint = {
  time: number;
  bid: number | null;
  ask: number | null;
};