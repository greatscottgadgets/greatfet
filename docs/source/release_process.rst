================================================
Release Process
================================================

This is the process for tagging and publishing a release. Change the release version number and paths as appropriate. Release version numbers are in the form ``YYYY.major.minor`` where ``YYYY`` is the year and ``major``, ``minor`` are the release numbers.


Create Environment
~~~~~~~~~~~~~~~~~~

.. code-block:: sh

    pyenv install 3.9
    pyenv virtualenv 3.9 gsg-release
    pyenv local gsg-release

    python -m pip install --upgrade pip
    pip install --upgrade setuptools setuptools_git_versioning build twine


Dependencies
~~~~~~~~~~~~

.. code-block:: sh

    pip install pyyaml wheel git-archive-all


Clone Repository
~~~~~~~~~~~~~~~~

.. code-block:: sh

    # clone repository
    git clone git@github.com:greatscottgadgets/greatfet.git greatfet.git

    # update submodules
    cd greatfet.git/
    git submodule update --init --recursive


Update ``RELEASENOTES.md`` from previous release
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  - update ``RELEASENOTES.md`` for the latest release


Prepare release
~~~~~~~~~~~~~~~

.. code-block:: sh

    RELEASE_VERSION=2024.0.1 make prepare_release


Push Release Tag
~~~~~~~~~~~~~~~~

.. code-block:: sh

    git tag -a v2024.0.1 -m 'release 2024.0.1'
    git push --tags


"Draft a new release" on github
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    - call it "release 2024.0.1"
    - paste release notes (just for this release, not previous)
    - upload .tar.xz and .zip files


Upload release to PyPI
~~~~~~~~~~~~~~~~~~~~~~

    python -m twine upload --repository pypi host-packages/*


Announce the release
~~~~~~~~~~~~~~~~~~~~

    - discord
    - greatfet mailing list
    - twitter
