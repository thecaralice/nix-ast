project(
  'nix-ast',
  'cpp',
  version: '0.1.0',
  default_options: ['warning_level=3', 'cpp_std=c++20'],
)

subdir('src')

# test
ver = '@0@.@1@'.format(nix_version[0], nix_version[1])
nix = find_program('nix')
jq = find_program('jq')
test_jq = files('tests/test.jq')[0]
foreach test: ['readme', 'inherit']
  out = custom_target(
    f'test-@test@-out',
    command: [
      nix,
      '--extra-experimental-features',
      'nix-command',
      '--option',
      'store',
      'dummy://',
      '--plugin-files',
      plugin,
      'ast',
      '--eval-store',
      'dummy://',
      '--file',
      files('tests' / 'input' / test + '.nix')[0],
    ],
    capture: true,
    output: f'test-@test@.out.json'
  )
  test(
    f'test-@test@',
    jq,
    args: [
      '--slurp',
      '--exit-status',
      '--from-file',
      test_jq,
      files('tests' / ver / test + '.json')[0],
      out,
    ],
  )
endforeach
