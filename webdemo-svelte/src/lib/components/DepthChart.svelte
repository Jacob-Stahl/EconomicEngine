<script lang="ts">
  import { renderDepthChart } from '../charts/renderers';
  import type { ObservationSnapshot } from '../sim/types';

  export let snapshot: ObservationSnapshot | null = null;

  let canvas: HTMLCanvasElement;
  let metaElement: HTMLParagraphElement;

  $: if (canvas && metaElement) {
    renderDepthChart({
      canvas,
      metaElement,
      snapshot,
    });
  }
</script>

<section class="chart-card">
  <div class="header">
    <p class="eyebrow">Order Book</p>
    <h2>Depth Snapshot</h2>
  </div>

  <canvas bind:this={canvas} width="960" height="320"></canvas>
  <p bind:this={metaElement} class="meta"></p>
</section>

<style>
  .chart-card {
    padding: 20px;
    border-radius: 24px;
    border: 1px solid rgba(30, 37, 32, 0.12);
    background: rgba(255, 252, 245, 0.88);
    box-shadow: 0 24px 60px rgba(33, 43, 36, 0.12);
  }

  .header {
    display: grid;
    gap: 6px;
    margin-bottom: 14px;
  }

  .eyebrow {
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
    font-size: 1.85rem;
  }

  .meta {
    margin: 12px 0 0;
    color: #59665e;
    font-family: 'SFMono-Regular', 'Menlo', monospace;
    font-size: 0.84rem;
    line-height: 1.5;
  }
</style>