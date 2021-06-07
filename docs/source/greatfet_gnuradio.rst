================================================
GreatFET and GNURadio
================================================

The GreatFET platform can easily interface with GNURadio -- and several gnuradio-companion blocks are available to make using GreatFET+GNURadio easier. These blocks include support for various neighbors (e.g. Gladiolus blocks for Software Defined Infrared) and a variety of other inputs and outputs, which are lumped into the 'Software Defined Everything' category.



Requirements
~~~~~~~~~~~~

You'll need an up-to-date GNURadio environment to use our blocks. This means:

- GNURadio >= 3.8, built with python3*
- python 3.6+

It's possible these blocks will work with GNURadio on lower python versions; but these aren't our development targets and aren't fully supported.



Adding our blocks to GNURadio-companion
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The easiest way to add our blocks to GNURadio is to modify our local GNURadio configuration. On Linux and macOS, this file is located at ``~/.gnuradio/config.conf``.

Open this file, and find the section that's headed ``[grc]``; or create the relevant section if it doesn't exist.

.. code-block:: sh

	[grc]
	<existing keys here>

We'll want to add the location of our blocks to the configuration file's ``local_blocks_path``. We can determine the location of our blocks using the ``gf info`` command:

.. code-block:: sh

	$ gf info --host
	Host tools info:
		host module version: 2019.5.1
		pygreat module version: 2019.9.1
		python version: 3.8.0 (default, Oct 23 2019, 18:51:26) 

		module path: /home/user/.local/lib/python3.8/site-packages/greatfet
		command path: /home/user/.local/lib/python3.8/site-packages/greatfet/commands
		gnuradio-companion block path: /home/user/.local/lib/python3.8/site-packages/greatfet/gnuradio

In the output above, the last line points out our GRC block path. We'll add this to the ``local_block_path`` entry in our configuration file:

.. code-block:: sh

	[grc]
	local_blocks_path = /home/user/.local/lib/python3.8/site-packages/greatfet/gnuradio

If you want to have multiple entries, here -- for example, if existing entries are already present -- you can separate multiple paths using colons, similar to Linux paths:

.. code-block:: sh

	[grc]
	local_blocks_path = /home/user/.local/lib/python3.8/site-packages/greatfet/gnuradio:/home/user/my_blocks

The next time you start GRC, you should see new headings (e.g. ``SDIR`` and ``Software Defined Everything``) in your list of available blocks.