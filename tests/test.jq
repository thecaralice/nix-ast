def assert(cond; msg): if cond | not then error(msg) end;
def cmp: 
	.fst as $fst
	| .snd as $snd
	| {fst: [], snd: [], diff: [], uncmp: []}
	| if $fst == $snd or (($fst | isnan) and ($snd | isnan)) then .
		elif ($fst | type) != ($snd | type) then .uncmp += [[]]
		elif [$fst, $snd] | map(scalars) | length == 2 then .diff += [[]]
		else
			($fst | keys | sort) as $fst_keys
			| ($snd | keys | sort) as $snd_keys
			| .fst += (($fst_keys - $snd_keys) | map([.]))
			| .snd += (($snd_keys - $fst_keys) | map([.]))
			| ($fst_keys - ($fst_keys - $snd_keys)) as $keys
			| reduce (
				$keys 
				| map(. as $key | {fst: $fst.[$key], snd: $snd.[$key]} | cmp | map_values(map([$key] + .))) 
				| .[]
			) as $cur (
				.;
				.fst += $cur.fst
				| .snd += $cur.snd
				| .diff += $cur.diff
				| .uncmp += $cur.uncmp
			)
	end
;

assert(length == 2; "usage: jq -esf test.jq exp.json out.json")
| .[0] as $exp
| .[1] as $out
| if $exp == $out then halt end
| {fst: $exp, snd: $out}
| cmp
| halt_error(6)
