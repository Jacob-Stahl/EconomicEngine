<script lang="ts">
  import { createEventDispatcher, onDestroy } from 'svelte';
  import WidgetFrame from './WidgetFrame.svelte';
  import type { WidgetFrameModel } from '../layout/types';

  export let widgets: WidgetFrameModel[] = [];
  export let columns = 12;
  export let rowHeight = 40;
  export let gap = 4;

  const minCanvasRows = 8;

  const dispatch = createEventDispatcher<{
    change: { widgets: WidgetFrameModel[] };
  }>();

  let shellElement: HTMLDivElement;
  let gridElement: HTMLDivElement;
  let activeId: string | null = null;
  let startPointer = { x: 0, y: 0 };
  let startWidget: WidgetFrameModel | null = null;
  let previewWidget: WidgetFrameModel | null = null;
  let isPanning = false;
  let panPointer = { x: 0, y: 0 };
  let panOffsetStart = { x: 0, y: 0 };
  let viewOffset = { x: 0, y: 0 };
  let canvasRows = minCanvasRows;

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

  function startPan(event: PointerEvent) {
    if (!shellElement || activeId) {
      return;
    }

    const target = event.target as HTMLElement | null;

    if (target?.closest('.widget-frame')) {
      return;
    }

    isPanning = true;
    panPointer = { x: event.clientX, y: event.clientY };
    panOffsetStart = { ...viewOffset };

    shellElement.setPointerCapture(event.pointerId);
    event.preventDefault();
  }

  function handlePanMove(event: PointerEvent) {
    if (!isPanning || !shellElement || !gridElement) {
      return;
    }

    const deltaX = event.clientX - panPointer.x;
    const deltaY = event.clientY - panPointer.y;

    viewOffset = clampViewOffset({
      x: panOffsetStart.x + deltaX,
      y: panOffsetStart.y + deltaY,
    });
  }

  function stopPan(event?: PointerEvent) {
    if (!isPanning || !shellElement) {
      return;
    }

    if (event && shellElement.hasPointerCapture(event.pointerId)) {
      shellElement.releasePointerCapture(event.pointerId);
    }

    isPanning = false;
  }

  function getColumnWidth() {
    const totalGapWidth = gap * (columns - 1);
    return (gridElement.clientWidth - totalGapWidth) / columns;
  }

  function clampViewOffset(offset: { x: number; y: number }) {
    if (!shellElement || !gridElement) {
      return offset;
    }

    const minX = Math.min(0, shellElement.clientWidth - gridElement.offsetWidth);
    const minY = Math.min(0, shellElement.clientHeight - gridElement.offsetHeight);

    return {
      x: clamp(offset.x, minX, 0),
      y: clamp(offset.y, minY, 0),
    };
  }

  function getCanvasRows() {
    if (widgets.length === 0) {
      return minCanvasRows;
    }

    const occupiedRows = Math.max(...widgets.map((widget) => widget.y + widget.height + 2));

    return Math.max(minCanvasRows, occupiedRows);
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

  $: canvasRows = getCanvasRows();
  $: if (shellElement && gridElement) {
    const clampedOffset = clampViewOffset(viewOffset);

    if (clampedOffset.x !== viewOffset.x || clampedOffset.y !== viewOffset.y) {
      viewOffset = clampedOffset;
    }
  }

  onDestroy(stopDrag);
</script>

<div
  bind:this={shellElement}
  class="grid-shell"
  class:panning={isPanning}
  role="application"
  aria-label="Pannable widget grid"
  on:pointerdown={startPan}
  on:pointermove={handlePanMove}
  on:pointerup={stopPan}
  on:pointercancel={stopPan}
>
  <div
    bind:this={gridElement}
    class="widget-grid"
    style={`--columns: ${columns}; --row-height: ${rowHeight}px; --gap: ${gap}px; --canvas-rows: ${canvasRows}; --column-width: 2.6667rem; --offset-x: ${viewOffset.x}px; --offset-y: ${viewOffset.y}px;`}
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
  .grid-shell {
    width: 100%;
    min-width: 0;
    overflow: hidden;
    max-height: 90vh;
    cursor: grab;
    touch-action: none;
  }

  .grid-shell.panning {
    cursor: grabbing;
  }

  .widget-grid {
    display: grid;
    grid-template-columns: repeat(var(--columns), minmax(var(--column-width), 1fr));
    grid-auto-rows: var(--row-height);
    gap: var(--gap);
    padding: var(--gap);
    min-width: calc((var(--columns) * var(--column-width)) + (var(--gap) * (var(--columns) + 1)));
    min-height: calc((var(--canvas-rows) * var(--row-height)) + (var(--gap) * (var(--canvas-rows) + 1)));
    align-content: start;
    transform: translate(var(--offset-x, 0px), var(--offset-y, 0px));
    will-change: transform;
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