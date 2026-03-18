import createEelib from '../generated/eelib.mjs';
import type { MainModule } from '../generated/eelib';

let modulePromise: Promise<MainModule> | null = null;

export function loadEelib(): Promise<MainModule> {
  if (!modulePromise) {
    modulePromise = createEelib() as Promise<MainModule>;
  }

  return modulePromise;
}