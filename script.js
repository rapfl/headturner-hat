const state = {
  mode: "closed",
  decay: 34,
  tone: 68,
  air: 62,
  drive: 41,
  audioContext: null,
  master: null,
  analyser: null,
  noiseBuffer: null,
  openVoiceGain: null,
  demoTimer: null,
  step: 0,
};

const controls = {
  trigger: document.querySelector("#trigger"),
  demo: document.querySelector("#demo"),
  scope: document.querySelector("#scope"),
  status: document.querySelector("#status-text"),
  sliders: {
    decay: document.querySelector("#decay"),
    tone: document.querySelector("#tone"),
    air: document.querySelector("#air"),
    drive: document.querySelector("#drive"),
  },
  outputs: {
    decay: document.querySelector("#decay-value"),
    tone: document.querySelector("#tone-value"),
    air: document.querySelector("#air-value"),
    drive: document.querySelector("#drive-value"),
  },
  modeButtons: [...document.querySelectorAll(".mode-toggle")],
};

function createNoiseBuffer(context) {
  const buffer = context.createBuffer(1, context.sampleRate * 0.6, context.sampleRate);
  const data = buffer.getChannelData(0);

  for (let i = 0; i < data.length; i += 1) {
    data[i] = (Math.random() * 2 - 1) * (1 - i / data.length);
  }

  return buffer;
}

async function ensureAudio() {
  if (state.audioContext) {
    if (state.audioContext.state === "suspended") {
      await state.audioContext.resume();
    }
    return state.audioContext;
  }

  const AudioContextClass = window.AudioContext || window.webkitAudioContext;
  const context = new AudioContextClass();
  const master = context.createGain();
  const limiter = context.createDynamicsCompressor();
  const analyser = context.createAnalyser();

  master.gain.value = 0.78;
  limiter.threshold.value = -14;
  limiter.knee.value = 18;
  limiter.ratio.value = 5;
  limiter.attack.value = 0.002;
  limiter.release.value = 0.16;
  analyser.fftSize = 512;

  master.connect(limiter);
  limiter.connect(analyser);
  analyser.connect(context.destination);

  state.audioContext = context;
  state.master = master;
  state.analyser = analyser;
  state.noiseBuffer = createNoiseBuffer(context);
  drawScope();

  return context;
}

function curveDrive(amount = 0) {
  const samples = 256;
  const curve = new Float32Array(samples);
  const drive = 1 + amount * 20;

  for (let i = 0; i < samples; i += 1) {
    const x = (i * 2) / (samples - 1) - 1;
    curve[i] = Math.tanh(x * drive);
  }

  return curve;
}

function mapDecay() {
  const base = 0.06 + (state.decay / 100) * 0.42;
  return state.mode === "open" ? base * 2.4 : base;
}

function mapTone() {
  return 6500 + (state.tone / 100) * 9500;
}

function mapAir() {
  return 4200 + (state.air / 100) * 4800;
}

function mapDrive() {
  return state.drive / 100;
}

function modeDescription() {
  const descriptors = [];

  descriptors.push(
    state.mode === "open"
      ? "Open mode lets the metallic tail breathe and hang a little longer."
      : "Closed mode keeps the hit clipped, tight, and ready for the grid."
  );

  if (state.decay > 65) {
    descriptors.push("Long decay adds more wash.");
  } else if (state.decay < 28) {
    descriptors.push("Short decay leaves more room for the kick.");
  } else {
    descriptors.push("Decay sits in the sharp middle.");
  }

  if (state.drive > 58) {
    descriptors.push("Drive pushes the edge into rougher crunch.");
  } else if (state.tone > 62) {
    descriptors.push("The top stays bright without turning to static.");
  } else {
    descriptors.push("Darker tone keeps it more machine than splash.");
  }

  controls.status.textContent = descriptors.join(" ");
}

function triggerHat(accent = 1) {
  const context = state.audioContext;
  if (!context) {
    return;
  }

  const now = context.currentTime;
  const decay = mapDecay();
  const tone = mapTone();
  const air = mapAir();
  const drive = mapDrive();

  if (state.mode === "closed" && state.openVoiceGain) {
    state.openVoiceGain.gain.cancelScheduledValues(now);
    state.openVoiceGain.gain.setTargetAtTime(0.0001, now, 0.015);
  }

  const voice = context.createGain();
  const voiceHighpass = context.createBiquadFilter();
  const voiceLowpass = context.createBiquadFilter();
  const saturator = context.createWaveShaper();

  voiceHighpass.type = "highpass";
  voiceHighpass.frequency.setValueAtTime(air, now);
  voiceHighpass.Q.value = 0.65;

  voiceLowpass.type = "lowpass";
  voiceLowpass.frequency.setValueAtTime(tone, now);
  voiceLowpass.Q.value = 0.2;

  saturator.curve = curveDrive(drive);
  saturator.oversample = "4x";

  voice.gain.setValueAtTime(0.0001, now);
  voice.gain.exponentialRampToValueAtTime(0.95 * accent, now + 0.002);
  voice.gain.exponentialRampToValueAtTime(0.0001, now + decay);

  voice.connect(voiceHighpass);
  voiceHighpass.connect(voiceLowpass);
  voiceLowpass.connect(saturator);
  saturator.connect(state.master);

  if (state.mode === "open") {
    state.openVoiceGain = voice;
  }

  const clusterFrequencies = [402, 533, 638, 810, 927, 1104];
  clusterFrequencies.forEach((frequency, index) => {
    const osc = context.createOscillator();
    const oscGain = context.createGain();
    const detuneSpread = (Math.random() - 0.5) * 16;

    osc.type = index % 2 === 0 ? "square" : "triangle";
    osc.frequency.setValueAtTime(frequency + detuneSpread, now);
    oscGain.gain.value = 0.12 / (1 + index * 0.08);

    osc.connect(oscGain);
    oscGain.connect(voice);
    osc.start(now);
    osc.stop(now + decay + 0.05);
  });

  const noise = context.createBufferSource();
  const noiseBandpass = context.createBiquadFilter();
  const noiseGain = context.createGain();

  noise.buffer = state.noiseBuffer;
  noiseBandpass.type = "bandpass";
  noiseBandpass.frequency.setValueAtTime(tone * 0.9, now);
  noiseBandpass.Q.value = 0.55;
  noiseGain.gain.value = state.mode === "open" ? 0.18 : 0.13;

  noise.connect(noiseBandpass);
  noiseBandpass.connect(noiseGain);
  noiseGain.connect(voice);
  noise.start(now);
  noise.stop(now + decay + 0.06);

  const click = context.createBufferSource();
  const clickFilter = context.createBiquadFilter();
  const clickGain = context.createGain();

  click.buffer = state.noiseBuffer;
  clickFilter.type = "highpass";
  clickFilter.frequency.setValueAtTime(Math.max(9000, tone * 0.9), now);
  clickGain.gain.setValueAtTime(0.0001, now);
  clickGain.gain.exponentialRampToValueAtTime(0.28 * accent, now + 0.001);
  clickGain.gain.exponentialRampToValueAtTime(0.0001, now + 0.012);

  click.connect(clickFilter);
  clickFilter.connect(clickGain);
  clickGain.connect(state.master);
  click.start(now);
  click.stop(now + 0.03);
}

function drawScope() {
  if (!state.analyser) {
    return;
  }

  const canvas = controls.scope;
  const ctx = canvas.getContext("2d");
  const data = new Uint8Array(state.analyser.frequencyBinCount);

  const render = () => {
    if (!state.analyser) {
      return;
    }

    state.analyser.getByteTimeDomainData(data);
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.fillStyle = "rgba(245, 239, 226, 0.98)";
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    ctx.strokeStyle = "rgba(17, 17, 17, 0.14)";
    ctx.beginPath();
    for (let x = 0; x <= canvas.width; x += 24) {
      ctx.moveTo(x, 0);
      ctx.lineTo(x, canvas.height);
    }
    for (let y = 0; y <= canvas.height; y += 20) {
      ctx.moveTo(0, y);
      ctx.lineTo(canvas.width, y);
    }
    ctx.stroke();

    ctx.strokeStyle = "#111111";
    ctx.lineWidth = 2;
    ctx.beginPath();
    for (let i = 0; i < data.length; i += 1) {
      const x = (i / (data.length - 1)) * canvas.width;
      const y = (data[i] / 255) * canvas.height;
      if (i === 0) {
        ctx.moveTo(x, y);
      } else {
        ctx.lineTo(x, y);
      }
    }
    ctx.stroke();
    requestAnimationFrame(render);
  };

  render();
}

function updateOutputs() {
  const decay = Math.round(mapDecay() * 1000);
  controls.outputs.decay.value = `${decay} ms`;
  controls.outputs.tone.value =
    state.tone < 35 ? "Dark" : state.tone < 70 ? "Bright" : "Cutting";
  controls.outputs.air.value =
    state.air < 35 ? "Tight" : state.air < 70 ? "Sharp" : "Glassy";
  controls.outputs.drive.value =
    state.drive < 30 ? "Clean" : state.drive < 65 ? "Crunch" : "Ripped";
  modeDescription();
}

function setMode(mode) {
  state.mode = mode;
  controls.modeButtons.forEach((button) => {
    button.classList.toggle("is-active", button.dataset.mode === mode);
  });
  updateOutputs();
}

function stepDemo() {
  const pattern = [
    { mode: "closed", accent: 0.85 },
    { mode: "closed", accent: 0.48 },
    { mode: "open", accent: 0.72 },
    { mode: "closed", accent: 0.56 },
    { mode: "closed", accent: 0.9 },
    { mode: "closed", accent: 0.5 },
    { mode: "closed", accent: 0.62 },
    { mode: "open", accent: 0.74 },
  ];

  const current = pattern[state.step % pattern.length];
  setMode(current.mode);
  triggerHat(current.accent);
  state.step += 1;
}

function toggleDemo() {
  if (state.demoTimer) {
    clearInterval(state.demoTimer);
    state.demoTimer = null;
    controls.demo.classList.remove("is-running");
    controls.demo.textContent = "Run demo";
    return;
  }

  state.step = 0;
  controls.demo.classList.add("is-running");
  controls.demo.textContent = "Stop demo";
  stepDemo();
  state.demoTimer = window.setInterval(stepDemo, 180);
}

controls.modeButtons.forEach((button) => {
  button.addEventListener("click", async () => {
    await ensureAudio();
    setMode(button.dataset.mode);
    triggerHat(0.92);
  });
});

Object.entries(controls.sliders).forEach(([key, slider]) => {
  slider.addEventListener("input", () => {
    state[key] = Number(slider.value);
    updateOutputs();
  });
});

controls.trigger.addEventListener("click", async () => {
  await ensureAudio();
  triggerHat(1);
});

controls.demo.addEventListener("click", async () => {
  await ensureAudio();
  toggleDemo();
});

window.addEventListener("keydown", async (event) => {
  if (event.code !== "Space" || event.repeat) {
    return;
  }

  event.preventDefault();
  await ensureAudio();
  triggerHat(1);
});

updateOutputs();
