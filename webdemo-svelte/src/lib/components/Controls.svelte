<script lang="ts">
  import type { SimulationParams } from '../sim/types';

  export let params: SimulationParams;
  export let disabled = false;
  export let onParamsChange: (nextParams: SimulationParams) => void;
  export let onRestart: () => void;

  function updateNumber(key: keyof SimulationParams, rawValue: string, minValue: number) {
    const parsed = Number.parseInt(rawValue, 10);
    const sanitized = Number.isNaN(parsed) ? params[key] : Math.max(minValue, parsed);

    onParamsChange({
      ...params,
      [key]: sanitized,
    });
  }
</script>

<section class="panel controls-panel">
  <div class="panel-header">
    <p class="panel-eyebrow">Inputs</p>
    <h2>Simulation Controls</h2>
    <p class="helper-copy">Adjust the active run live, or restart to rebuild the market from tick zero.</p>
  </div>

  <div class="grid">
    <label>
      <span>Consumer Hunger Delay Mean</span>
      <input
        type="number"
        min="0"
        step="1"
        value={params.hungerDelayMean}
        on:change={(event) => updateNumber('hungerDelayMean', event.currentTarget.value, 0)}
      />
    </label>

    <label>
      <span>Consumer Hunger Delay Std Dev</span>
      <input
        type="number"
        min="0"
        step="1"
        value={params.hungerDelayStd}
        on:change={(event) => updateNumber('hungerDelayStd', event.currentTarget.value, 0)}
      />
    </label>

    <label>
      <span>Consumer Max Price</span>
      <input
        type="number"
        min="0"
        step="1"
        value={params.maxPrice}
        on:change={(event) => updateNumber('maxPrice', event.currentTarget.value, 0)}
      />
    </label>

    <label>
      <span>Number of Consumers</span>
      <input
        type="number"
        min="1"
        step="1"
        value={params.numConsumers}
        on:change={(event) => updateNumber('numConsumers', event.currentTarget.value, 1)}
      />
    </label>

    <label>
      <span>Producer Preferred Price</span>
      <input
        type="number"
        min="0"
        step="1"
        value={params.producerPrice}
        on:change={(event) => updateNumber('producerPrice', event.currentTarget.value, 0)}
      />
    </label>
  </div>

  <button type="button" on:click={onRestart} disabled={disabled}>
    Restart Simulation
  </button>
</section>

<style>
  .panel {
    padding: 20px;
    border-radius: 24px;
    border: 1px solid rgba(30, 37, 32, 0.12);
    background: rgba(255, 252, 245, 0.88);
    box-shadow: 0 24px 60px rgba(33, 43, 36, 0.12);
  }

  .panel-header {
    display: grid;
    gap: 6px;
    margin-bottom: 14px;
  }

  .panel-eyebrow {
    margin: 0;
    color: #9a6b12;
    font-family: 'SFMono-Regular', 'Menlo', monospace;
    font-size: 0.78rem;
    letter-spacing: 0.12em;
    text-transform: uppercase;
  }

  h2 {
    margin: 0;
    font-family: Charter, 'Iowan Old Style', 'Palatino Linotype', serif;
    font-size: 1.9rem;
    line-height: 1.08;
  }

  .helper-copy {
    margin: 0;
    color: #59665e;
    line-height: 1.55;
  }

  .grid {
    display: grid;
    gap: 12px;
  }

  label {
    display: grid;
    gap: 6px;
  }

  span {
    color: #59665e;
    font-size: 0.92rem;
  }

  input {
    width: 100%;
    padding: 0.8rem 0.9rem;
    border: 1px solid rgba(30, 37, 32, 0.12);
    border-radius: 14px;
    background: rgba(255, 255, 255, 0.96);
    color: #1e2520;
  }

  input:focus {
    outline: 2px solid rgba(15, 118, 110, 0.28);
    outline-offset: 2px;
  }

  button {
    margin-top: 14px;
    width: 100%;
    padding: 0.95rem 1rem;
    border: none;
    border-radius: 999px;
    background: linear-gradient(135deg, #0f766e, #155e75);
    color: #f8fafc;
    cursor: pointer;
    font-weight: 600;
  }

  button:disabled {
    cursor: not-allowed;
    opacity: 0.65;
  }
</style>