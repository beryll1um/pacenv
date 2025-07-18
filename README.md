# pacenv (experimental)
One more front-end for the ALPM library, implementing creation and management
of lightweight GNU/Linux environments.

## Warning
This is an experimental utility and lacks a set of security measures such
as GNUPG keyring management, profiling, etc. Use it at your own risk!

## Usage
In demonstration purposes, let's install `figlet` and
configure environment to use it.
```bash
cat << EOF > figenv.json
{
	"name": "figenv",
	"syncdbs": {
		"world": [
			"https://mirrors.dotsrc.org/artix-linux/repos/world/os/x86_64/"
		]
	},
	"deps": [ "figlet>=2.2.5-6" ]
}
EOF
pacenv -c figenv.json .figenv
source .figenv/usr/local/bin/activate
export FIGLET_FONTDIR=$PWD/.figenv/usr/share/figlet/fonts/
figlet pacenv
deactivate_figenv
```

## Contribution
I'll be happy to implement any improvements you suggest as soon as possible,
but I have other things to do in my life, so if you want to contribute too,
please do! I'll try to review them as soon as possible along with community!
