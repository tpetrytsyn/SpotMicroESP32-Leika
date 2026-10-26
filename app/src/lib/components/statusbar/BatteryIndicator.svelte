<script lang="ts">
    import { Battery, Battery1, Battery2, Battery3, Battery4, BatteryExclamation } from '../icons'

    interface Props {
        voltage?: number
    }

    let { voltage = 0 }: Props = $props()

    // 2S LiPo: 8.4 V full, 7.4 V nominal, 7.0 V the working cutoff under load (docs/wiring.md)
    const CUTOFF = 7.0

    const getBatteryIcon = () => {
        if (voltage <= 0) return Battery
        if (voltage >= 8.0) return Battery4
        if (voltage >= 7.6) return Battery3
        if (voltage >= 7.3) return Battery2
        if (voltage >= CUTOFF) return Battery1
        return BatteryExclamation
    }

    const getColor = () => {
        if (voltage <= 0) return ''
        if (voltage < CUTOFF) return 'text-error'
        if (voltage < 7.3) return 'text-warning'
        return ''
    }

    const BatteryIcon = $derived(getBatteryIcon())
    const color = $derived(getColor())
    const label = $derived(voltage > 0 ? `${voltage.toFixed(2)} V` : 'No reading')
</script>

<div class="tooltip tooltip-left" data-tip={'Pack ' + label}>
    <div class="flex items-center gap-1 {color}">
        <BatteryIcon class="h-7 w-7" />
        {#if voltage > 0}
            <span class="text-sm tabular-nums">{voltage.toFixed(2)} V</span>
        {/if}
    </div>
</div>
