import type { RefObject } from "react";
import { type TooltipPage } from "./components/tooltip/Tooltip";

export interface OnboardingRefs {
  algoMenuPanelRef: RefObject<HTMLElement>;
  settingsPanelRef: RefObject<HTMLElement>;
  navbarRef: RefObject<HTMLElement>;
  canvasRef: RefObject<HTMLElement>;
  codePanelRef: RefObject<HTMLElement>;
  infoPanel1Ref: RefObject<HTMLElement>;
  infoPanel2Ref: RefObject<HTMLElement>;
}

export function createTooltipPages(ref: OnboardingRefs): TooltipPage[] {
  const {
    algoMenuPanelRef,
    settingsPanelRef,
    navbarRef,
    canvasRef,
    codePanelRef,
    infoPanel1Ref,
    infoPanel2Ref,
  } = ref;

  return [
    {
      title: "Algorithm List",
      text: [
        "This panel contains all the algorithms you can try out in AlgoPlex.",
        "In this demo, only graph algorithms are available.",
      ],
      newLoc: true,
      tooltipTarget: algoMenuPanelRef,
      tooltipPos: { x: 250, y: -100 },
      arrowRotation: 90,
      arrowRelDistance: { x: -5, y: 10 },
    },
    // c>
    //   title: "Algorithm List",
    //   text: [
    //     "For now, you may pick one algorithm from the Graph/Tree list.",
    //     "Pick one.",
    //   ],
    //   newLoc: false,
    //   tooltipTarget: algoMenuPanelRef,
    //   tooltipPos: { x: 250, y: -100 },
    //   arrowRotation: 90,
    //   arrowRelDistance: { x: -5, y: 10 },
    // },
    {
      title: "Scene Canvas",
      text: [
        "This is where the nodes and edges are displayed.",
        "You can hold left mouse button to pan, and scroll to zoom in and out.",
      ],
      newLoc: true,
      tooltipTarget: canvasRef,
      tooltipPos: { x: -800, y: -200 },
      arrowRotation: -90,
      arrowRelDistance: { x: 25, y: 10 },
    },
    {
      title: "Controls Panel",
      text: [
        "This Panel has all the input modes for the canvas, and settings for the algorithm scene.",
        "All the controls and settings for the selected algorithm are here.",
      ],
      newLoc: true,
      tooltipTarget: settingsPanelRef,
      tooltipPos: { x: -700, y: 0 },
      arrowRotation: -90,
      arrowRelDistance: { x: 25, y: 10 },
    },

    {
      title: "Controls Panel",
      text: [
        "Create a few nodes by selecting Node and Create. Then clicking on the canvas to place them.",
        "To connect them, Select Edge, then right click on a node in the canvas to start drawing an edge.",
      ],
      newLoc: false,
      tooltipTarget: settingsPanelRef,
      subHighlightTarget: canvasRef,
      targetCOverride: {
        right: 1,
        left: 1,
        top: 1,
        bottom: 0,
        x: 0,
        y: 0,
        height: 0,
        width: 0,
      },
      tooltipPos: { x: -200, y: 250 },
      arrowRotation: 200,
      arrowRelDistance: { x: 10, y: -3 },
    },
    {
      title: "Controls Panel",
      text: ["You can set the root node by using the dropdown menu."],
      newLoc: false,
      tooltipTarget: settingsPanelRef,
      tooltipPos: { x: -550, y: -350 },
      arrowRotation: -90,
      arrowRelDistance: { x: 25, y: 5 },
    },
    {
      title: "Start The Algorithm",
      text: [
        "Press Start to begin the traversal, then Step to go through a single iteration.",
      ],
      targetCOverride: {
        right: 0.8,
        left: 1,
        top: 1,
        bottom: 1,
        x: 0,
        y: 0,
        height: 0,
        width: 0,
      },
      newLoc: true,
      tooltipTarget: navbarRef,
      tooltipPos: { x: 0, y: 100 },
      arrowRotation: 180,
      arrowRelDistance: { x: 10, y: -3 },
    },
    {
      title: "DFS Stack. ",
      text: [
        "(Queue view not implemented yet)",
        "This is the stack view.",
        "Here you will be able to see the nodes which will get pushed into the stack array.",
      ],
      newLoc: true,
      tooltipTarget: infoPanel1Ref,
      tooltipPos: { x: 200, y: -500 },
      arrowRotation: 45,
      arrowRelDistance: { x: -5, y: 20 },
    },
    {
      title: "Graph View",
      text: [
        "This is the adjacency matrix.",
        "Here you will be able to see the nodes and the edges connecting them.",
      ],
      newLoc: true,
      tooltipTarget: infoPanel2Ref,
      tooltipPos: { x: -600, y: -600 },
      arrowRotation: -45,
      arrowRelDistance: { x: 22.5, y: 20 },
    },
    {
      title: "Code Editor",
      text: [
        "This is where the algorithm's code is displayed and then highlighted step wise during execution",
      ],
      newLoc: true,
      tooltipTarget: codePanelRef,
      tooltipPos: { x: 0, y: -700 },
      arrowRotation: 0,
      arrowRelDistance: { x: 10, y: 25 },
    },
  ];
}

