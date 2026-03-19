<script lang="ts">
  import { createEventDispatcher, onDestroy } from 'svelte';
  import WidgetFrame from './WidgetFrame.svelte';
  import type { WidgetFrameModel } from '../layout/types';

  export let widgets: WidgetFrameModel[] = [];
  export let columns = 12;
  export let rowHeight = 120;
  export let gap = 12;

  const dispatch = createEventDispatcher<{
    change: { widgets: WidgetFrameModel[] };
  }>();

  let gridElement: HTMLDivElement;
  let activeId: string | null = null;
  let startPointer = { x: 0, y: 0 };
  let startWidget: WidgetFrameModel | null = null;
  let previewWidget: WidgetFrameModel | null = null;

  function clamp(value: number, min: number, max: number) {
    return Math.min(Math.max(value, min), max);
  }

  function overlaps(a: WidgetFrameModel, b: WidgetFrameModel) {
    return (
      a.x < b.x + b.width &&
      a.x + a.width > b.x &&
      a.y < b.y + b.height &&
      a.y + a.height > b.y
    );
  }

  function canPlace(candidate: WidgetFrameModel) {
    if (candidate.x < 0 || candidate.y < 0) {
      return false;
    }

    if (candidate.x + candidate.width > columns) {
      return false;
    }

    return widgets.every((widget) => widget.id === candidate.id || !overlaps(widget, candidate));
  }

  function emitChange(next: WidgetFrameModel[]) {
    dispatch('change', { widgets: next });
  }

  function removeWidget(id: string) {
    emitChange(widgets.filter((widget) => widget.id !== id));
  }

  function getColumnWidth() {
    const totalGapWidth = gap * (columns - 1);
    return (gridElement.clientWidth - totalGapWidth) / columns;
  }

  function startDrag(event: CustomEvent<{ id: string; clientX: number; clientY: number }>) {
    activeId = event.detail.id;
    startPointer = { x: event.detail.clientX, y: event.detail.clientY };
    startWidget = widgets.find((widget) => widget.id === activeId) ?? null;
    previewWidget = startWidget;

    window.addEventListener('pointermove', handlePointerMove);
    window.addEventListener('pointerup', stopDrag);
    window.addEventListener('pointercancel', stopDrag);
  }

  function handlePointerMove(event: PointerEvent) {
    if (!activeId || !startWidget || !gridElement || gridElement.clientWidth === 0) {
      return;
    }

    const stepX = getColumnWidth() + gap;
    const stepY = rowHeight + gap;
    const deltaX = Math.round((event.clientX - startPointer.x) / stepX);
    const deltaY = Math.round((event.clientY - startPointer.y) / stepY);

    const candidate: WidgetFrameModel = {
      ...startWidget,
      x: clamp(startWidget.x + deltaX, 0, columns - startWidget.width),
      y: Math.max(0, startWidget.y + deltaY),
    };

    previewWidget = candidate;
  }

  function stopDrag() {
    if (previewWidget && canPlace(previewWidget)) {
      const droppedWidget = previewWidget;

      emitChange(widgets.map((widget) => (widget.id === droppedWidget.id ? droppedWidget : widget)));
    }

    activeId = null;
    startWidget = null;
    previewWidget = null;
    window.removeEventListener('pointermove', handlePointerMove);
    window.removeEventListener('pointerup', stopDrag);
    window.removeEventListener('pointercancel', stopDrag);
  }

  function widgetStyle(widget: WidgetFrameModel) {
    const zIndex = widget.id === activeId ? 1 : 0;

    return `grid-column: ${widget.x + 1} / span ${widget.width}; grid-row: ${widget.y + 1} / span ${widget.height}; position: relative; z-index: ${zIndex};`;
  }

  function outlineStyle(widget: WidgetFrameModel) {
    return `grid-column: ${widget.x + 1} / span ${widget.width}; grid-row: ${widget.y + 1} / span ${widget.height};`;
  }

  onDestroy(stopDrag);
</script>

<div class="grid-shell">
  <div
    bind:this={gridElement}
    class="widget-grid"
    style={`--columns: ${columns}; --row-height: ${rowHeight}px; --gap: ${gap}px;`}
  >
    {#if activeId && previewWidget}
      <div class="drag-outline" style={outlineStyle(previewWidget)} aria-hidden="true"></div>
    {/if}

    {#each widgets as widget (widget.id)}
      <WidgetFrame
        widget={widget}
        dragging={widget.id === activeId}
        style={widgetStyle(widget)}
        on:remove={(event) => removeWidget(event.detail.id)}
        on:grab={startDrag}
      >
        <slot {widget}>
          <div class="placeholder">
            <p>{widget.name}</p>
            <p>{widget.width} x {widget.height}</p>
          </div>
        </slot>
      </WidgetFrame>
    {/each}
  </div>
</div>

<style>
  .widget-grid {
    display: grid;
    grid-template-columns: repeat(var(--columns), minmax(0, 1fr));
    grid-auto-rows: var(--row-height);
    gap: var(--gap);
    padding: var(--gap);
    background-image:
      repeating-linear-gradient(
        to right,
        transparent 0,
        transparent calc((100% - (var(--gap) * (var(--columns) - 1))) / var(--columns) + var(--gap) - 1px),
        rgba(0, 0, 0, 0.18) calc((100% - (var(--gap) * (var(--columns) - 1))) / var(--columns) + var(--gap) - 1px),
        rgba(0, 0, 0, 0.18) calc((100% - (var(--gap) * (var(--columns) - 1))) / var(--columns) + var(--gap))
      ),
      repeating-linear-gradient(
        to bottom,
        transparent 0,
        transparent calc(var(--row-height) + var(--gap) - 1px),
        rgba(0, 0, 0, 0.18) calc(var(--row-height) + var(--gap) - 1px),
        rgba(0, 0, 0, 0.18) calc(var(--row-height) + var(--gap))
      );
  }

  .drag-outline {
    border: 2px dashed rgba(0, 0, 0, 0.65);
    pointer-events: none;
    z-index: 2;
  }
</style>