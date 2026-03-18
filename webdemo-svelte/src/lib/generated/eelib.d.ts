// TypeScript bindings for emscripten-generated code.  Automatically generated at compile time.
interface WasmModule {
}

type EmbindString = ArrayBuffer|Uint8Array|Uint8ClampedArray|Int8Array|string;
export interface ClassHandle {
  isAliasOf(other: ClassHandle): boolean;
  delete(): void;
  deleteLater(): this;
  isDeleted(): boolean;
  // @ts-ignore - If targeting lower than ESNext, this symbol might not exist.
  [Symbol.dispose](): void;
  clone(): this;
}
export interface OrdTypeValue<T extends number> {
  value: T;
}
export type OrdType = OrdTypeValue<1>|OrdTypeValue<2>|OrdTypeValue<3>|OrdTypeValue<4>;

export interface SideValue<T extends number> {
  value: T;
}
export type Side = SideValue<1>|SideValue<2>;

export interface tick extends ClassHandle {
  raw(): number;
}

export interface VectorString extends ClassHandle, Iterable<string> {
  size(): number;
  get(_0: number): string | undefined;
  push_back(_0: EmbindString): void;
  resize(_0: number, _1: EmbindString): void;
  set(_0: number, _1: EmbindString): boolean;
}

export interface VectorPriceBin extends ClassHandle, Iterable<PriceBin> {
  push_back(_0: PriceBin): void;
  resize(_0: number, _1: PriceBin): void;
  size(): number;
  get(_0: number): PriceBin | undefined;
  set(_0: number, _1: PriceBin): boolean;
}

export type Depth = {
  bidBins: VectorPriceBin,
  askBins: VectorPriceBin
};

export interface MapStringSpread extends ClassHandle {
  keys(): VectorString;
  size(): number;
  get(_0: EmbindString): Spread | undefined;
  set(_0: EmbindString, _1: Spread): void;
}

export interface MapStringDepth extends ClassHandle {
  keys(): VectorString;
  size(): number;
  get(_0: EmbindString): Depth | undefined;
  set(_0: EmbindString, _1: Depth): void;
}

export type Observation = {
  time: tick,
  assetSpreads: MapStringSpread,
  assetOrderDepths: MapStringDepth
};

export interface ConsumerManager extends ClassHandle {
  changeMaxPrice(_0: number, _1: number): void;
  changeNumAgents(_0: number): void;
  changeHungerDelay(_0: number, _1: number): void;
}

export interface ProducerManager extends ClassHandle {
  changePreferedPrice(_0: number, _1: number): void;
  changeNumAgents(_0: number): void;
}

export interface ABM extends ClassHandle {
  getLatestObservation(): Observation;
  simStep(): void;
  getNumAgents(): number;
}

export type Spread = {
  bidsMissing: boolean,
  asksMissing: boolean,
  highestBid: number,
  lowestAsk: number
};

export type PriceBin = {
  price: number,
  totalQty: number
};

interface EmbindModule {
  OrdType: {MARKET: OrdTypeValue<1>, LIMIT: OrdTypeValue<2>, STOP: OrdTypeValue<3>, STOPLIMIT: OrdTypeValue<4>};
  Side: {BUY: SideValue<1>, SELL: SideValue<2>};
  tick: {
    new(_0: number): tick;
  };
  VectorString: {
    new(): VectorString;
  };
  VectorPriceBin: {
    new(): VectorPriceBin;
  };
  MapStringSpread: {
    new(): MapStringSpread;
  };
  MapStringDepth: {
    new(): MapStringDepth;
  };
  ConsumerManager: {};
  ProducerManager: {};
  ABM: {
    new(): ABM;
  };
  createConsumerManager(_0: ABM | null, _1: EmbindString, _2: EmbindString): ConsumerManager | null;
  createProducerManager(_0: ABM | null, _1: EmbindString, _2: EmbindString): ProducerManager | null;
}

export type MainModule = WasmModule & EmbindModule;
export default function MainModuleFactory (options?: unknown): Promise<MainModule>;
