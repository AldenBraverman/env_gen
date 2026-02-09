import { laneParamIds } from "./params";

export interface SectionConfig {
  id: string;
  title: string;
  paramIds: string[];
}

export const SECTIONS: SectionConfig[] = [
  {
    id: "GAIN",
    title: "Gain",
    paramIds: ["inputGain", "outputGain", "dryPass"],
  },
  {
    id: "ENVELOPE",
    title: "Envelope",
    paramIds: ["numLanes", ...laneParamIds],
  },
];
