Name:           obsidian-fault-script
Version:        1.0.97
Release:        1%{?dist}
Summary:        Obsidian Fault Script self-hosted compiler and standard library
License:        BSL-1.0
URL:            https://github.com/Samwns/Obsidian-Fault-Script
Source0:        %{name}-%{version}.tar.gz
Requires:       clang

%description
Obsidian Fault Script (OFS) language toolchain: command wrapper, self-hosted native compiler, runtime library, standard library, and compiler sources.

%prep

%build

%install
mkdir -p %{buildroot}
tar xzf %{SOURCE0} -C %{buildroot}

%files
/usr/bin/ofs
/usr/bin/ofscc
/usr/lib/ofs/ofscc
/usr/lib/ofs/libofs_runtime.a
/usr/share/ofs/stdlib
/usr/share/ofs/ofscc-src
/usr/share/doc/obsidian-fault-script
/usr/share/licenses/obsidian-fault-script/LICENSE
