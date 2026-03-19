<script lang="ts">
  import { createEventDispatcher } from 'svelte';
  import type { WidgetFrameModel } from '../layout/types';

  export let widget: WidgetFrameModel;
  export let dragging = false;
  export let style = '';

  const dispatch = createEventDispatcher<{
    remove: { id: string };
    grab: { id: string; clientX: number; clientY: number };
  }>();

  function handlePointerDown(event: PointerEvent) {
    event.preventDefault();

    dispatch('grab', {
      id: widget.id,
      clientX: event.clientX,
      clientY: event.clientY,
    });
  }

  function removeWidget() {
    dispatch('remove', { id: widget.id });
  }
</script>

<section class="widget-frame" class:dragging style={style}>
  <button type="button" class="drag-handle" aria-label={`Move ${widget.name}`} on:pointerdown={handlePointerDown}>
    <span class="handle-mark" aria-hidden="true"></span>
    <span class="handle-label">Drag</span>
  </button>

  <header class="widget-header">
    <div class="title-group">
      <p class="eyebrow">Widget</p>
      <h2>{widget.name}</h2>
    </div>

    <button
      type="button"
      class="remove-button"
      aria-label={`Remove ${widget.name}`}
      on:click={removeWidget}
    >
      Remove
    </button>
  </header>

  <div class="widget-content">
    <slot />
  </div>
</section>

<style>
  .widget-frame {
    display: grid;
    grid-template-rows: auto auto 1fr;
    min-height: 100%;
    border: 1px solid rgba(0, 0, 0, 0.35);
    user-select: none;
  }

  .drag-handle {
    display: grid;
    justify-items: center;
    gap: 0.125rem;
    border: 0;
    border-bottom: 1px solid rgba(0, 0, 0, 0.2);
    padding: 0.375rem 0.5rem;
    background: transparent;
    text-align: center;
    touch-action: none;
  }

  .handle-mark {
    width: 2.5rem;
    height: 0.375rem;
    background-image: radial-gradient(circle, currentColor 1px, transparent 1.5px);
    background-size: 0.5rem 0.375rem;
    background-repeat: repeat-x;
    background-position: center;
    opacity: 0.75;
  }

  .handle-label {
    font-size: 0.75rem;
  }

  .widget-header {
    display: flex;
    justify-content: space-between;
    align-items: start;
    gap: 0.5rem;
    padding: 0.5rem;
  }

  .title-group {
    min-width: 0;
  }

  .remove-button {
    border: 0;
    background: transparent;
  }

  .widget-content {
    padding: 0 0.5rem 0.5rem;
  }
</style>