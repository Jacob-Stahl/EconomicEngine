import type {
  ABM,
  ClassHandle,
  ConsumerManager,
  Depth,
  MainModule,
  Observation,
  ProducerManager,
  Spread,
  VectorPriceBin,
} from '../generated/eelib';
import { loadEelib } from '../wasm/eelib';
import type {
  DepthBinSnapshot,
  DepthSnapshot,
  ObservationSnapshot,
  SpreadSnapshot,
} from './types';


function safeDelete(handle: Partial<ClassHandle> | null | undefined) {
  if (!handle || typeof handle.delete !== 'function') {
    return;
  }

  try {
    handle.delete();
  } catch {
    // Ignore repeated or failed cleanup during boilerplate teardown.
  }
}

function readBins(bins: VectorPriceBin): DepthBinSnapshot[] {
  const snapshots: DepthBinSnapshot[] = [];

  for (let index = 0; index < bins.size(); index += 1) {
    const bin = bins.get(index);
    if (!bin) {
      continue;
    }

    snapshots.push({
      price: bin.price,
      totalQty: bin.totalQty,
    });
  }

  return snapshots;
}

function toSpreadSnapshot(spread: Spread | undefined): SpreadSnapshot | null {
  if (!spread) {
    return null;
  }

  return {
    bidsMissing: spread.bidsMissing,
    asksMissing: spread.asksMissing,
    highestBid: spread.highestBid,
    lowestAsk: spread.lowestAsk,
  };
}

function toDepthSnapshot(depth: Depth | undefined): DepthSnapshot | null {
  if (!depth) {
    return null;
  }

  try {
    return {
      bidBins: readBins(depth.bidBins),
      askBins: readBins(depth.askBins),
    };
  } finally {
    safeDelete(depth.bidBins);
    safeDelete(depth.askBins);
  }
}