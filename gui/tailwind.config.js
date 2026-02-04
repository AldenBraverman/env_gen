/** @type {import('tailwindcss').Config} */
export default {
  darkMode: "class",
  content: ["./index.html", "./src/**/*.{js,ts,jsx,tsx}"],
  theme: {
    extend: {
      colors: {
        background: "hsl(var(--background))",
        foreground: "hsl(var(--foreground))",
        muted: "hsl(var(--muted))",
        accent: "hsl(var(--accent))",
        border: "hsl(var(--border))",
        primary: "hsl(var(--primary))",
        ring: "hsl(var(--ring))",
        input: "hsl(var(--input))",
        card: "hsl(var(--card))",
        popover: "hsl(var(--popover))",
        "popover-foreground": "hsl(var(--popover-foreground))",
        "primary-foreground": "hsl(var(--primary-foreground))",
      },
    },
  },
  plugins: [],
};
