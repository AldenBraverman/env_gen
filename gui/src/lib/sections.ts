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
    paramIds: [
      "lane1_step0", "lane1_step1", "lane1_step2", "lane1_step3",
      "lane1_step4", "lane1_step5", "lane1_step6", "lane1_step7",
      "lane1_step8", "lane1_step9", "lane1_step10", "lane1_step11",
      "lane1_step12", "lane1_step13", "lane1_step14", "lane1_step15",
      "lane1_attack", "lane1_hold", "lane1_decay", "lane1_rate", "lane1_destination", "lane1_amount",
    ],
  },
];
